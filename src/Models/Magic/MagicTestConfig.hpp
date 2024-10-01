#pragma once

#include <Frame.h>

#include <Samplers.h>
#include <QuantumState.h>

#define MTC_T_DOPED_CLIFFORD 0
#define MTC_RANDOM_HAAR 1
#define MTC_T_GATES 2

#define MTC_STATEVECTOR 0
#define MTC_MPS 1

class MagicTestConfig : public dataframe::Config {
  public:
    double phi;
    size_t system_size;
    bool apply_random_clifford;

    int state_type;
    size_t num_t_gates;

    int quantum_state_type;
    size_t bond_dimension;

    QuantumStateSampler sampler;

    MagicTestConfig(dataframe::Params &params) : dataframe::Config(params), sampler(params) {
      phi = dataframe::utils::get<double>(params, "phi");
      system_size = dataframe::utils::get<int>(params, "system_size", 1);

      state_type = dataframe::utils::get<int>(params, "state_type", MTC_T_DOPED_CLIFFORD);
      if (state_type == MTC_T_GATES) {
        num_t_gates = dataframe::utils::get<int>(params, "num_t_gates", 1);
      }
      apply_random_clifford = dataframe::utils::get<int>(params, "apply_random_clifford", 1);

      quantum_state_type = dataframe::utils::get<int>(params, "quantum_state_type", MTC_STATEVECTOR);
      if (quantum_state_type == MTC_MPS) {
        bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 16);
      }
    }

    virtual dataframe::DataSlide compute(uint32_t num_threads) override {
      auto start = std::chrono::high_resolution_clock::now();

      Eigen::Matrix2cd T;
      T << 1.0, 0.0, 0.0, std::exp(std::complex<double>(0.0, phi));

      std::shared_ptr<QuantumState> state;
      if (quantum_state_type == MTC_STATEVECTOR) {
        state = std::make_shared<Statevector>(system_size);
      } else {
        state = std::make_shared<MatrixProductState>(system_size, bond_dimension);
      }

      thread_local std::random_device gen;
      std::minstd_rand rng(gen());

      QuantumCircuit qc(system_size);
      if (state_type == MTC_T_DOPED_CLIFFORD) {
        qc.add_gate("h", {0});
        qc.add_gate(T, {0});
      } else if (state_type == MTC_RANDOM_HAAR) {
        qc.append(generate_haar_circuit(system_size, system_size, false));
      } else if (state_type == MTC_T_GATES) {
        std::vector<size_t> qubits(system_size);
        std::iota(qubits.begin(), qubits.end(), 0);
        std::shuffle(qubits.begin(), qubits.end(), rng);
        qubits.resize(num_t_gates);

        for (auto q : qubits) {
          qc.add_gate("h", {static_cast<uint32_t>(q)});
          qc.add_gate("t", {static_cast<uint32_t>(q)});
        }
      }

      if (apply_random_clifford) {
        thread_local std::random_device gen;
        std::minstd_rand rng(gen());
        for (size_t k = 0; k < 3; k++) {
          for (size_t i = 0; i < system_size/2 - 1; i++) {
            uint32_t q1 = (k % 2) ? 2*i : 2*i + 1;
            uint32_t q2 = q1 + 1;

            qc.append(random_clifford(2, rng), {q1, q2});
          }
        }
      }

      state->evolve(qc);

      dataframe::DataSlide slide;
      dataframe::data_t samples;

      sampler.add_samples(samples, state);
      slide.add_samples(samples);
      slide.push_samples(samples);

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
      slide.add_data("time");
      slide.push_samples_to_data("time", duration.count());

      return slide;
    }

    virtual std::shared_ptr<Config> clone() override {
      return std::make_shared<MagicTestConfig>(params);
    }
};
