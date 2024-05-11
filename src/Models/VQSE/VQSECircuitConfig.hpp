#pragma once

#include "VQSE.hpp"
#include <Frame.h>
#include <cstdint>
#include <random>

// Ansatz types
#define HARDWARE_EFFICIENT_ANSATZ 0
#define DEFAULT_ANSATZ HARDWARE_EFFICIENT_ANSATZ

// Target types
#define HAAR_RANDOM 0
#define REAL_DENSITY 1

// Gradient settings
#define FINITE_DIFFERENCE_GRADIENT 0
#define PARAMETER_SHIFT_GRADIENT 1

// Parameter initialization settings
#define ZERO_PARAMS 0
#define RANDOM_PARAMS 1
#define DEFAULT_PARAMS_INIT RANDOM_PARAMS

class VQSECircuitConfig : public dataframe::Config {
  public:
    VQSECircuitConfig(dataframe::Params &params) : dataframe::Config(params) {
      seed = dataframe::utils::get<int>(params, "seed", -1);
      if (seed == -1) {
        thread_local std::random_device rd;
        seed = rd();
      }

      rng = std::mt19937(seed);

      // VQSE configuration
      num_qubits = dataframe::utils::get<int>(params, "num_qubits");			

      hamiltonian_type = dataframe::utils::get<int>(params, "hamiltonian_type", VQSE_ADAPTIVE_HAMILTONIAN);
      num_iterations_per_layer = dataframe::utils::get<int>(params, "num_iterations_per_layer");
      update_frequency = dataframe::utils::get<int>(params, "update_frequency", 20);
      sampling_type = dataframe::utils::get<int>(params, "sampling_type", VQSE_EXACT_SAMPLING);
      num_shots = dataframe::utils::get<int>(params, "num_shots", 1024);
      params_init = dataframe::utils::get<int>(params, "params_init", RANDOM_PARAMS);

      // Gradient configuration	
      gradient_type = dataframe::utils::get<int>(params, "gradient_type", FINITE_DIFFERENCE_GRADIENT);
      noisy_gradients = dataframe::utils::get<int>(params, "noisy_gradients", false);
      gradient_noise = dataframe::utils::get<double>(params, "gradient_noise", 1e-5);

      // Target configuration
      mzr_prob = dataframe::utils::get<double>(params, "mzr_prob");	
      target_depth = dataframe::utils::get<int>(params, "target_depth", 1);

      // Ansatz configuration
      entangling_gate = dataframe::utils::get<std::string>(params, "entangling_gate", "cz");
      rotation_gates = parse_rotation_gates(dataframe::utils::get<std::string>(params, "rotation_gates", "Rx, Ry, Rz"));

      // Data collection
      record_err = dataframe::utils::get<int>(params, "record_err", true);
      record_fidelity = dataframe::utils::get<int>(params, "record_fidelity", true);
    }

    virtual dataframe::DataSlide compute(uint32_t num_threads) override {
      Eigen::setNbThreads(num_threads);

      auto start = std::chrono::high_resolution_clock::now();

      dataframe::DataSlide slide;

      if (record_err) {
        slide.add_data("rel_err");
        slide.add_data("abs_err");
      }

      if (record_fidelity) {
        slide.add_data("fidelity");
      }

      Statevector target_state(num_qubits);
      auto callback = [this, &slide, &target_state](const std::vector<double>& params) {
        if (record_err) {
          auto [rel_err, abs_err] = vqse.error(target_state);
          slide.push_samples_to_data("rel_err", rel_err);
          slide.push_samples_to_data("abs_err", abs_err);
        }

        if (record_fidelity) {
          auto fidelity = vqse.fidelity(target_state, params);
          slide.push_samples_to_data("fidelity", vqse.fidelity(target_state, params));
        }
      };


      ADAMOptimizer optimizer(std::nullopt, std::nullopt, std::nullopt, std::nullopt, gradient_type, noisy_gradients, gradient_noise);


      target = prepare_target_circuit();
      std::vector<bool> outputs;

      QuantumCircuit trained_ansatz(num_qubits);

      for (uint32_t d = 1; d <= target_depth; d++) {
        QuantumCircuit ansatz(num_qubits);
        ansatz.append(trained_ansatz);    // Section which has already been bound
        ansatz.append(prepare_ansatz(2)); // Variational section
        QuantumCircuit target_d = prepare_target_circuit_to_depth(target, d);

        outputs.push_back(rng() % 2);


        target_state = Statevector(num_qubits);
        target_state.evolve(target_d, outputs);

        std::vector<double> params = initialize_params(ansatz.num_params());
        vqse = VQSE(ansatz, 1, num_iterations_per_layer, hamiltonian_type, update_frequency, sampling_type, num_shots, optimizer);
        vqse.optimize(target_state, params, callback);

        trained_ansatz = ansatz.bind_params(vqse.params);
      }

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
      slide.add_data("time");
      slide.push_samples_to_data("time", duration.count());
  
      slide.add_data("circuit_depth");
      slide.push_samples_to_data("circuit_depth", (target.length() - target_depth)/(num_qubits/2));

      return slide;
    }

    virtual std::shared_ptr<Config> clone() override {
      return std::make_shared<VQSECircuitConfig>(params);
    }

  private:
    static bool printed_ompi_threads;

    int seed;
    std::mt19937 rng;

    uint32_t num_qubits;

    // VQSE settings
    uint32_t hamiltonian_type;
    uint32_t num_iterations_per_layer;
    uint32_t update_frequency;

    uint32_t gradient_type;
    bool noisy_gradients;
    double gradient_noise;
    uint32_t params_init;

    bool sampling_type;
    uint32_t num_shots;

    // Target settings
    double mzr_prob;
    uint32_t target_depth;

    // Ansatz settings
    uint32_t ansatz_depth;
    std::string entangling_gate;
    std::vector<std::string> rotation_gates;


    QuantumCircuit target;
    VQSE vqse;

    bool record_err;
    bool record_fidelity;
    bool record_energy_levels;
    uint32_t num_energy_levels;

    double randf() {
      return double(rng())/double(RAND_MAX);
    }

    int rand() {
      return rng();
    }

    static std::vector<std::string> parse_rotation_gates(const std::string& s) {
      std::vector<std::string> gates;
      std::stringstream ss(s);
      std::string token;

      while (std::getline(ss, token, ',')) {
        gates.push_back(dataframe::utils::strip(token));
      }

      return gates;
    }

    QuantumCircuit prepare_target_circuit() {
      QuantumCircuit circuit(num_qubits);
      
      uint32_t depth = 0;
      while (depth < target_depth) {
        for (uint32_t i = 0; i < target_depth; i++) {
          for (uint32_t q = 0; q < num_qubits/2; q++) {
            auto [q1, q2] = get_targets(i, q, num_qubits);

            circuit.add_gate(haar_unitary(2, rng), {q1, q2});
          }
        }

        for (uint32_t q = 0; q < num_qubits; q++) {
          if (randf() < mzr_prob) {
            circuit.add_measurement(q);
            depth++;
            if (depth == target_depth) {
              break;
            }
          }
        }
      }

      return circuit;
    }

    QuantumCircuit prepare_target_circuit_to_depth(const QuantumCircuit& target, uint32_t depth) const {
      QuantumCircuit circuit(num_qubits);

      uint32_t d = 0;
      uint32_t i = 0;
      while (d < depth) {
        Instruction inst = target.instructions[i];
        circuit.append(inst);
        
        if (inst.index() == 1) {
          d++;
        }

        i++;
      }

      return circuit;
    }

    QuantumCircuit prepare_ansatz(uint32_t ansatz_depth) const {
        return hardware_efficient_ansatz(num_qubits, ansatz_depth, rotation_gates, entangling_gate);
    }

    std::vector<double> initialize_params(uint32_t num_params) {
      std::vector<double> params(num_params, 0.0);

      if (params_init == ZERO_PARAMS) { // All params init to 0
                                        // Do nothing
      } else if (params_init == RANDOM_PARAMS) { // Randomly on [0, 2*pi]
        for (uint32_t i = 0; i < num_params; i++) {
          params[i] = 2*PI*randf();
        }
      }

      return params;
    }
};


bool VQSECircuitConfig::printed_ompi_threads = false;
