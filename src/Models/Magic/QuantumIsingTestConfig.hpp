#pragma once

#include <Frame.h>

#include <Samplers.h>
#include <QuantumState.h>

class QuantumIsingTestConfig : public dataframe::Config {
  public:
    size_t system_size;
    size_t bond_dimension;
    double h;
    int seed;

    QuantumStateSampler quantum_sampler;
    EntropySampler entropy_sampler;

    QuantumIsingTestConfig(dataframe::Params &params) : dataframe::Config(params), quantum_sampler(params), entropy_sampler(params) {
      seed = dataframe::utils::get<int>(params, "seed", 0);
      system_size = dataframe::utils::get<int>(params, "system_size", 1);
      bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 64);
      h = dataframe::utils::get<double>(params, "h");
    }

    virtual dataframe::DataSlide compute(uint32_t num_threads) override {
      auto start = std::chrono::high_resolution_clock::now();

      std::shared_ptr<MatrixProductState> state = std::make_shared<MatrixProductState>(MatrixProductState::ising_ground_state(system_size, h, bond_dimension));

      dataframe::DataSlide slide;
      dataframe::data_t samples;

      auto surface = state->get_entropy_surface<double>(1);

      slide.add_data("surface", surface.size());
      slide.push_samples_to_data("surface", surface);

      quantum_sampler.add_samples(samples, state);
      entropy_sampler.add_samples(samples, state);
      slide.add_samples(samples);
      slide.push_samples(samples);

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
      slide.add_data("time");
      slide.push_samples_to_data("time", duration.count());

      return slide;
    }

    virtual std::shared_ptr<Config> clone() override {
      return std::make_shared<QuantumIsingTestConfig>(params);
    }
};
