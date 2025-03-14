#pragma once

#include "VQSE.hpp"
#include <Frame.h>

// Ansatz types
#define HARDWARE_EFFICIENT_ANSATZ 0
#define DEFAULT_ANSATZ HARDWARE_EFFICIENT_ANSATZ

// Target types
#define HAAR_RANDOM 0
#define REAL_DENSITY 1
#define DEFAULT_TARGET HAAR_RANDOM
#define DEFAULT_NUM_MEASUREMENTS 1
#define DEFAULT_POST_MEASUREMENT_LAYERS 0
#define DEFAULT_DENSITY_MATRIX_TARGET true

// VQSE settings
#define DEFAULT_SIMULATED_SAMPLING false
#define DEFAULT_NUM_SHOTS 1024
#define DEFAULT_UPDATE_FREQUENCY 30

// Gradient settings
#define DEFAULT_GRADIENT_TYPE 0
#define DEFAULT_NOISY_GRADIENTS false
#define DEFAULT_GRADIENT_NOISE 0.01

// Parameter initialization settings
#define ZERO_PARAMS 0
#define RANDOM_PARAMS 1
#define DEFAULT_PARAMS_INIT RANDOM_PARAMS

// Data recording settings
#define DEFAULT_RECORD_ERR true
#define DEFAULT_RECORD_FIDELITY false
#define DEFAULT_RECORD_ENERGY_LEVELS false
#define DEFAULT_NUM_ENERGY_LEVELS 1

class VQSEConfig {
  public:
    VQSEConfig(dataframe::ExperimentParams &params) {
      // VQSE configuration
      num_qubits = dataframe::utils::get<int>(params, "num_qubits");			
      m = dataframe::utils::get<int>(params, "m");
      hamiltonian_type = dataframe::utils::get<int>(params, "hamiltonian_type", VQSE_ADAPTIVE_HAMILTONIAN);
      num_iterations = dataframe::utils::get<int>(params, "num_iterations");
      update_frequency = dataframe::utils::get<int>(params, "update_frequency", DEFAULT_UPDATE_FREQUENCY);
      simulated_sampling = dataframe::utils::get<int>(params, "simulated_sampling", DEFAULT_SIMULATED_SAMPLING);
      num_shots = dataframe::utils::get<int>(params, "num_shots", DEFAULT_NUM_SHOTS);
      params_init = dataframe::utils::get<int>(params, "params_init", DEFAULT_PARAMS_INIT);

      // Gradient configuration	
      gradient_type = dataframe::utils::get<int>(params, "gradient_type", DEFAULT_GRADIENT_TYPE);
      noisy_gradients = dataframe::utils::get<int>(params, "noisy_gradients", DEFAULT_NOISY_GRADIENTS);
      gradient_noise = dataframe::utils::get<double>(params, "gradient_noise", DEFAULT_GRADIENT_NOISE);

      // Tardataframe::utils::get configuration
      target_type = dataframe::utils::get<int>(params, "target_type", DEFAULT_TARGET);
      density_matrix_target = dataframe::utils::get<int>(params, "density_matrix_target", DEFAULT_DENSITY_MATRIX_TARGET);
      target_depth = dataframe::utils::get<int>(params, "target_depth");
      post_measurement_layers = dataframe::utils::get<int>(params, "post_measurement_layers", DEFAULT_POST_MEASUREMENT_LAYERS);
      num_measurements = dataframe::utils::get<int>(params, "num_measurements", DEFAULT_NUM_MEASUREMENTS);

      // Ansatz configuration
      ansatz_type = dataframe::utils::get<int>(params, "ansatz_type", DEFAULT_ANSATZ);
      ansatz_depth = dataframe::utils::get<int>(params, "ansatz_depth");
      entangling_gate = dataframe::utils::get<std::string>(params, "entangling_gate", "cz");
      rotation_gates = parse_rotation_gates(dataframe::utils::get<std::string>(params, "rotation_gates", "Rx, Ry, Rz"));

      // Data collection
      record_err = dataframe::utils::get<int>(params, "record_err", DEFAULT_RECORD_ERR);
      record_fidelity = dataframe::utils::get<int>(params, "record_fidelity", DEFAULT_RECORD_FIDELITY);
      record_energy_levels = dataframe::utils::get<int>(params, "record_energy_levels", DEFAULT_RECORD_ENERGY_LEVELS);
      num_energy_levels = dataframe::utils::get<int>(params, "num_energy_levels", DEFAULT_NUM_ENERGY_LEVELS);

      if (num_energy_levels > (1u << num_qubits)) {
        throw std::invalid_argument("Not enough qubits to compute requested energy levels.");
      }
    }

    void add_est_eigenvalues(dataframe::DataSlide& slide) {	
      slide.add_data("est_eigenvalues", vqse.eigenvalue_estimates.size());
      slide.push_samples_to_data("est_eigenvalues", vqse.eigenvalue_estimates);

      //slide.add_data("bitstrings", vqse.bitstring_estimates.size());
      //slide.push_samples_to_data("bitstrings", vqse.bitstring_estimates);
    }

    void add_true_eigensystem(dataframe::DataSlide &slide) {
      auto [eigenvalues, eigenvectors] = vqse.true_eigensystem(target);

      //slide.add_data("true_eigenvalues", eigenvalues.size());
      //slide.push_samples_to_data("true_eigenvalues", eigenvalues);

      for (uint32_t i = 0; i < m ; i++) {
        // TODO add back in
        //std::string state_real = "state_" + std::to_string(i) + "r";
        //std::string state_imag = "state_" + std::to_string(i) + "i";
        //slide.add_data(state_real);
        //slide.add_data(state_imag);

        //uint32_t s = (1u << num_qubits);
        //for (uint32_t j = 0; j < s; j++) {
        //  slide.push_samples_to_data(state_real, eigenvectors(i, j).real());
        //  slide.push_samples_to_data(state_imag, eigenvectors(i, j).imag());
        //}
      }


    }

    dataframe::DataSlide compute(uint32_t num_threads) {
      Eigen::setNbThreads(num_threads);

      if (!VQSEConfig::printed_ompi_threads) {
        //std::cout << fmt::format("OMP_NUM_THREADS for Eigen parallelization: {}\n", Eigen::nbThreads);
        VQSEConfig::printed_ompi_threads = true;
      }

      auto start = std::chrono::high_resolution_clock::now();

      ansatz = VQSEConfig::prepare_ansatz(num_qubits, ansatz_depth, ansatz_type, rotation_gates, entangling_gate);
      ADAMOptimizer optimizer(std::nullopt, std::nullopt, std::nullopt, std::nullopt, gradient_type, noisy_gradients, gradient_noise);
      vqse = VQSE(ansatz, m, num_iterations, hamiltonian_type, update_frequency, simulated_sampling, num_shots, optimizer);

      // Randomly select qubits to measure
      std::vector<uint32_t> qubits(num_qubits);
      std::iota(qubits.begin(), qubits.end(), 0);
      thread_local std::random_device rd;
      std::mt19937 gen(rd());
      std::shuffle(qubits.begin(), qubits.end(), gen);
      std::vector<uint32_t> measured_qubits(qubits.begin(), qubits.begin() + num_measurements);

      target = VQSEConfig::prepare_target(num_qubits, target_depth, target_type, post_measurement_layers, measured_qubits, density_matrix_target);

      dataframe::DataSlide slide;

      if (record_err) {
        slide.add_data("rel_err");
        slide.add_data("abs_err");
      }

      if (record_fidelity) {
        slide.add_data("fidelity");
      }

      if (record_energy_levels) {
        for (uint32_t i = 0; i < num_energy_levels; i++) {
          slide.add_data("local_energy_" + std::to_string(i));
          slide.add_data("global_energy_" + std::to_string(i));
          slide.add_data("total_energy_" + std::to_string(i));
        }

      }

      auto callback = [this, &slide](const std::vector<double>& params) {
        if (record_err) {
          auto [rel_err, abs_err] = vqse.error(target);
          slide.push_samples_to_data("rel_err", rel_err);
          slide.push_samples_to_data("abs_err", abs_err);
        }

        if (record_fidelity) {
          slide.push_samples_to_data("fidelity", vqse.fidelity(target, params));
        }

        if (record_energy_levels) {
          slide.push_samples_to_data("local_energy", vqse.get_local_energy_levels(num_energy_levels));
          slide.push_samples_to_data("global_energy", vqse.get_global_energy_levels(num_energy_levels));
          slide.push_samples_to_data("total_energy", vqse.get_energy_levels(num_energy_levels));
        }
      };

      std::vector<double> initial_params = initialize_params();
      vqse.optimize(target, initial_params, callback);


      // Optimization done; add results
      slide.add_data("final_parameters", vqse.params.size());
      slide.push_samples_to_data("final_parameters", vqse.params);

      add_true_eigensystem(slide);
      add_est_eigenvalues(slide);

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
      slide.add_data("time");
      slide.push_samples_to_data("time", duration.count());

      return slide;
    }

  private:
    static bool printed_ompi_threads;

    uint32_t num_qubits;

    uint32_t m;
    uint32_t hamiltonian_type;
    uint32_t num_iterations;
    uint32_t update_frequency;

    uint32_t gradient_type;
    bool noisy_gradients;
    double gradient_noise;

    uint32_t params_init;

    bool density_matrix_target;
    bool simulated_sampling;
    uint32_t num_shots;

    uint32_t target_type;
    uint32_t target_depth;
    uint32_t post_measurement_layers;
    uint32_t num_measurements;

    uint32_t ansatz_type; 
    uint32_t ansatz_depth;
    std::string entangling_gate;
    std::vector<std::string> rotation_gates;


    VQSE vqse;
    QuantumCircuit ansatz;
    target_t target;

    bool record_err;
    bool record_fidelity;
    bool record_energy_levels;
    uint32_t num_energy_levels;

    static std::vector<std::string> parse_rotation_gates(const std::string& s) {
      std::vector<std::string> gates;
      std::stringstream ss(s);
      std::string token;

      while (std::getline(ss, token, ',')) {
        gates.push_back(dataframe::utils::strip(token));
      }

      return gates;
    }

    static target_t prepare_target(
        uint32_t num_qubits, 
        uint32_t target_depth, 
        uint32_t target_type, 
        uint32_t post_measurement_layers,
        const std::vector<uint32_t>& measured,
        bool density_matrix_target
        ) {
      if (target_type == HAAR_RANDOM) {
        QuantumCircuit qc = generate_haar_circuit(num_qubits, target_depth);

        for (auto const& q : measured) {
          qc.add_measurement(Measurement::computational_basis(q));
        }

        qc.append(generate_haar_circuit(num_qubits, post_measurement_layers));

        if (density_matrix_target) {
          return DensityMatrix(qc);
        } else {
          return qc;
        }
      } else if (target_type == REAL_DENSITY) { // According to https://arxiv.org/pdf/2004.01372.pdf
        if (!density_matrix_target) {
          throw std::invalid_argument("If target is REAL_DENSITY, density_matrix_target must be True.");
        }

        uint32_t num_ancilla = 4;
        QuantumCircuit qc(num_qubits + num_ancilla);

        for (uint32_t i = 0; i < num_qubits; i++) {
          for (uint32_t j = 0; j < num_ancilla; j++) {
            std::vector<uint32_t> targets{i, num_qubits + j};
            qc.add_gate(random_real_unitary(), targets);
          }
        }

        std::vector<uint32_t> ancilla(num_ancilla);
        std::iota(ancilla.begin(), ancilla.end(), num_qubits);

        DensityMatrix rho(qc);
        rho = rho.partial_trace_density_matrix(ancilla);
        return rho;
      }

      return prepare_target(num_qubits, target_depth, DEFAULT_TARGET, post_measurement_layers, measured, density_matrix_target);
    }

    static QuantumCircuit prepare_ansatz(
        uint32_t num_qubits, 
        uint32_t ansatz_depth, 
        uint32_t ansatz_type, 
        const std::vector<std::string>& rotation_gates,
        const std::string& entangling_gate
        ) {
      if (ansatz_type == HARDWARE_EFFICIENT_ANSATZ) {
        return hardware_efficient_ansatz(num_qubits, ansatz_depth, rotation_gates, entangling_gate);
      }

      return prepare_ansatz(num_qubits, ansatz_depth, DEFAULT_ANSATZ, rotation_gates, entangling_gate);
    }

    std::vector<double> initialize_params() const {
      uint32_t num_params = ansatz.num_params();

      std::vector<double> params(num_params, 0.0);

      thread_local std::random_device rd;
      std::mt19937 gen(rd());

      if (params_init == ZERO_PARAMS) { // All params init to 0
                                        // Do nothing
      } else if (params_init == RANDOM_PARAMS) { // Randomly on [0, 2*pi]
        for (uint32_t i = 0; i < num_params; i++) {
          params[i] = 2*PI*double(gen())/double(RAND_MAX);
        }
      }

      return params;
    }
};


bool VQSEConfig::printed_ompi_threads = false;
