#pragma once

#include <Simulator.hpp>
#include <QuantumState.h>
#include <QuantumStateSampler.hpp>

class RandomCircuitSamplingSimulator : public Simulator {
	private:
		uint32_t system_size;

		double mzr_prob;
		int evolution_type;

		bool offset;

		EntropySampler entropy_sampler;
		QuantumStateSampler prob_sampler;

		void full_haar();
		void brickwork_haar();

	public:
		std::shared_ptr<Statevector> state;

		RandomCircuitSamplingSimulator(Params &params, uint32_t num_threads);

		virtual void timesteps(uint32_t num_steps) override;

		virtual data_t take_samples() override;
};