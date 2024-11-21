#pragma once

#include <Frame.h>

#include <Samplers.h>
#include <QuantumState.h>

#include <unsupported/Eigen/KroneckerProduct>

class HalfSystemQuantumIsingConfig {
  public:
    size_t system_size;
    size_t bond_dimension;
    size_t num_sweeps;

    std::string sre_method;
    size_t num_samples;
    size_t equilibration_timesteps;

    double h;

    HalfSystemQuantumIsingConfig(dataframe::Params &params) {
      system_size = dataframe::utils::get<int>(params, "system_size", 1);
      bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 64);
      h = dataframe::utils::get<double>(params, "h");

      sre_method = dataframe::utils::get<std::string>(params, "sre_method", "montecarlo");
      num_samples = dataframe::utils::get<int>(params, "num_samples", 0);
      equilibration_timesteps = dataframe::utils::get<int>(params, "equilibration_timesteps", 0);
      num_sweeps = dataframe::utils::get<int>(params, "num_sweeps", 10);
    }

    std::vector<double> take_sre_samples(auto& state) {
      std::vector<PauliAmplitude> samples;
      auto prob = [](double t) -> double { return t*t; };
      if (sre_method == "montecarlo") {
        samples = state.sample_paulis_montecarlo(num_samples, equilibration_timesteps, prob);
      } else if (sre_method == "exhaustive") {
        samples = state.sample_paulis_exhaustive();
      } else if (sre_method == "exact") {
        samples = state.sample_paulis_exact(num_samples, prob);
      } else {
        samples = state.sample_paulis(num_samples);
      }

      std::vector<double> t;
      for (const auto& [P, tp] : samples) {
        t.push_back(tp);
      }

      return t;
    }

    magic_t get_magic_samples(auto& state, const std::vector<uint32_t>& qubitsA, const std::vector<uint32_t>& qubitsB) {
      if (sre_method == "montecarlo") {
        return state.magic_mutual_information_montecarlo(qubitsA, qubitsB, num_samples, equilibration_timesteps);
      } else if (sre_method == "exhaustive") {
        return state.magic_mutual_information_exhaustive(qubitsA, qubitsB);
      } else if (sre_method == "exact") {
        return state.magic_mutual_information_exact(qubitsA, qubitsB, num_samples);
      } else {
        return state.magic_mutual_information(qubitsA, qubitsB, num_samples);
      }
    }

    dataframe::DataSlide compute(uint32_t num_threads) {
      auto start = std::chrono::high_resolution_clock::now();

      std::vector<uint32_t> qubits(system_size/2);
      std::iota(qubits.begin(), qubits.end(), 0);

      std::vector<uint32_t> qubitsA(system_size/4);
      std::iota(qubitsA.begin(), qubitsA.end(), 0);

      std::vector<uint32_t> qubitsB(system_size/4);
      std::iota(qubitsB.begin(), qubitsB.end(), 3*system_size/4);

      std::vector<uint32_t> qubitsAB(qubitsA.begin(), qubitsA.end());
      qubitsAB.insert(qubitsAB.end(), qubitsB.begin(), qubitsB.end());

      size_t n = system_size;
      auto vector_complement = [n](const std::vector<uint32_t>& v) {
        std::vector<bool> mask(n, true);
        for (const auto q : v) {
          mask[q] = false;
        }

        std::vector<uint32_t> v_;
        for (size_t i = 0; i < n; i++) {
          if (mask[i]) {
            v_.push_back(i);
          }
        }

        return v_;
      };

      MatrixProductState mps = MatrixProductState::ising_ground_state(system_size, h, bond_dimension, 1e-8, num_sweeps);
      MatrixProductOperator mpo2 = mps.partial_trace(qubits);
      MatrixProductOperator mpoAB = mps.partial_trace(vector_complement(qubitsAB));
      MatrixProductOperator mpoA = mps.partial_trace(vector_complement(qubitsA));
      MatrixProductOperator mpoB = mps.partial_trace(vector_complement(qubitsA));
      MatrixProductOperator mpo0 = mps.partial_trace({});

      //auto t = take_sre_samples(mps);
      auto t2 = take_sre_samples(mpo2);
      auto tAB = take_sre_samples(mpoAB);
      auto tA = take_sre_samples(mpoA);
      auto tB = take_sre_samples(mpoB);
      auto t0 = take_sre_samples(mpo0);

      auto magic_samples = get_magic_samples(mps, qubitsA, qubitsB);
      
      dataframe::data_t samples;
      //dataframe::utils::emplace(samples, "t", t);
      dataframe::utils::emplace(samples, "t2", t2);
      dataframe::utils::emplace(samples, "tAB", tAB);
      dataframe::utils::emplace(samples, "tA", tA);
      dataframe::utils::emplace(samples, "tB", tB);
      dataframe::utils::emplace(samples, "t0", t0);

      dataframe::utils::emplace(samples, "m", magic_samples);

      dataframe::DataSlide slide;
      slide.add_data(samples);
      slide.push_samples_to_data(samples);

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
      slide.add_data("time");
      slide.push_samples_to_data("time", duration.count());

      return slide;
    }
};
