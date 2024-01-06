#pragma once

#include <Simulator.hpp>
#include <QuantumState.h>
#include <Samplers.h>

class RandomCircuitSamplingSimulator : public dataframe::Simulator {
	private:
		uint32_t system_size;

		double mzr_prob;
		int evolution_type;

		bool offset;

		EntropySampler entropy_sampler;
		QuantumStateSampler prob_sampler;

		void full_haar();
		void brickwork_haar();
    void random_haar();

	public:
		std::shared_ptr<Statevector> state;

		RandomCircuitSamplingSimulator(dataframe::Params &params, uint32_t num_threads);

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::data_t take_samples() override;
};
