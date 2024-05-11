#pragma once

#include <Frame.h>

class CliffordClusteringConfig : public dataframe::Config {
  public:
    CliffordClusteringConfig(dataframe::Params &params) : dataframe::Config(params) {
      num_qubits = dataframe::utils::get<int>(params, "system_size");
      mzr_prob = dataframe::utils::get<double>(params, "mzr_prob");

      equilibration_timesteps = dataframe::utils::get<int>(params, "equilibration_timesteps");
      sampling_timesteps = dataframe::utils::get<int>(params, "sampling_timesteps");
      measurement_freq = dataframe::utils::get<int>(params, "measurement_freq");

      num_copies = dataframe::utils::get<int>(params, "num_copies");

      int seed = dataframe::utils::get<int>(params, "seed", -1);
      if (seed == -1) {
        thread_local std::random_device rd;
        rng.seed(rd());
      } else {
        rng.seed(seed);
      }
    }

    static std::vector<double> average_ensemble_distance(
        const std::vector<QuantumGraphState>& ensemble1, 
        const std::vector<QuantumGraphState>& ensemble2, 
        bool skip_same_index=false
    ) {
      std::vector<double> samples;

      for (size_t i = 0; i < ensemble1.size(); i++) {
        for (size_t j = 0; j < ensemble2.size(); j++) {
          if (skip_same_index && (i == j)) {
            continue;
          }

          double d = static_cast<double>(ensemble1[i].distance(ensemble2[j]));
          samples.push_back(d);
        }
      }

      return samples;
    }

    virtual dataframe::DataSlide compute(uint32_t num_threads) override {
      auto start = std::chrono::high_resolution_clock::now();

      size_t num_samples = sampling_timesteps/measurement_freq;
      std::vector<std::vector<QuantumGraphState>> states(num_copies);
      for (size_t i = 0; i < num_copies; i++) {
        states[i] = std::vector<QuantumGraphState>(num_samples);

        QuantumGraphState g(num_qubits, rng());
        rc_timesteps(g, equilibration_timesteps, mzr_prob);

        for (size_t j = 0; j < num_samples; j++) {
          rc_timesteps(g, measurement_freq, mzr_prob);
          states[i][j] = QuantumGraphState(g);
        }
      }

      std::vector<double> intracopy_samples;

      for (size_t i = 0; i < num_copies; i++) {
        auto samples = CliffordClusteringConfig::average_ensemble_distance(states[i], states[i], true);
        intracopy_samples.insert(intracopy_samples.end(), samples.begin(), samples.end());
      }

      std::vector<double> intercopy_samples;
      for (size_t i = 0; i < num_copies; i++) {
        for (size_t j = 0; j < num_copies; j++) {
          if (i == j) {
            continue;
          }

          auto samples = CliffordClusteringConfig::average_ensemble_distance(states[i], states[j], false);
          intercopy_samples.insert(intercopy_samples.end(), samples.begin(), samples.end());
        }
      }

      dataframe::DataSlide slide;
      
      slide.add_samples("intracopy_distance");
      slide.push_samples("intracopy_distance", std::vector<std::vector<double>>{intracopy_samples});

      slide.add_samples("intercopy_distance");
      slide.push_samples("intercopy_distance", std::vector<std::vector<double>>{intercopy_samples});

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
      slide.add_data("time");
      slide.push_samples_to_data("time", duration.count());

      return slide;
    }

    virtual std::shared_ptr<Config> clone() override {
      return std::make_shared<CliffordClusteringConfig>(params);
    }

  private:
    std::minstd_rand rng;
    uint32_t num_qubits;
    double mzr_prob;

    uint32_t equilibration_timesteps;
    uint32_t sampling_timesteps;
    uint32_t measurement_freq;
    uint32_t num_copies;

    static void rc_timesteps(QuantumGraphState& state, uint32_t num_steps, double mzr_prob) {
      uint32_t num_qubits = state.num_qubits;
      uint32_t num_gates = num_qubits / 2;
      bool offset_layer = false;

      std::vector<uint32_t> qubits(2);
      std::iota(qubits.begin(), qubits.end(), 0);
      for (uint32_t i = 0; i < num_steps; i++) {
        for (uint32_t j = 0; j < num_gates; j++) {
          uint32_t offset = offset_layer ? 2*j : 2*j + 1;

          std::vector<uint32_t> offset_qubits(qubits);
          std::transform(offset_qubits.begin(), offset_qubits.end(), 
              offset_qubits.begin(), [num_qubits, offset](uint32_t x) { return (x + offset) % num_qubits; } );
          state.random_clifford(offset_qubits);
        }

        for (size_t j = 0; j < num_qubits; j++) {
          if (state.randf() < mzr_prob) {
            state.mzr(j);
          }
        }

        offset_layer = !offset_layer;
      }
    }
};

