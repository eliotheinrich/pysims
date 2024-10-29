#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

class MatrixProductSimulator : public dataframe::Simulator {
	private:
		uint32_t system_size;
		double mzr_prob;
    double xx_prob;
    size_t bond_dimension;

		InterfaceSampler interface_sampler;
    QuantumStateSampler quantum_sampler;

    bool offset;

    void measure(size_t i, size_t j) {
      if (randf() < mzr_prob) {
        // Do something
        if (randf() < xx_prob) {
          PauliString XX(2);
          XX.set_x(0, 1);
          XX.set_x(1, 1);
          state->measure(XX, {static_cast<uint32_t>(i), static_cast<uint32_t>(j)});
        } else {
          state->mzr(i);
        }
      }
    }

    void unitary(size_t i, size_t j) {
      Eigen::Matrix4cd gate = haar_unitary(2, rng);
      state->evolve(gate, {static_cast<uint32_t>(i), static_cast<uint32_t>(j)});
    }

	public:
    std::shared_ptr<MatrixProductState> state;
		MatrixProductSimulator(dataframe::Params &params, uint32_t num_threads) : Simulator(params), interface_sampler(params), quantum_sampler(params){
      system_size = dataframe::utils::get<int>(params, "system_size");
      mzr_prob = dataframe::utils::get<double>(params, "mzr_prob");
      xx_prob = dataframe::utils::get<double>(params, "xx_prob");
      bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 32);

      offset = false;

      state = std::make_shared<MatrixProductState>(system_size, bond_dimension);
      state->seed(rand());
    }

		virtual void timesteps(uint32_t num_steps) override {
      for (size_t t = 0; t < num_steps; t++) {
        for (size_t i = 0; i < system_size/2 - offset; i++) {
          size_t q1 = 2*i + offset;
          size_t q2 = 2*i + 1 + offset;
          unitary(q1, q2);
        }

        for (size_t i = 0; i < system_size-1; i++) {
          // do measurements
          measure(i, i+1);
        }

        // Maybe mzr rightmost edge
        if (randf() < mzr_prob && randf() > xx_prob) {
          state->mzr(system_size - 1);
        }

        offset = !offset;
      }
    }

		virtual dataframe::data_t take_samples() override {
      dataframe::data_t samples;
      std::vector<int> surface = state->get_entropy_surface<int>(1u);
      interface_sampler.add_samples(samples, surface);

      quantum_sampler.add_samples(samples, state);

      return samples;
    }
};
