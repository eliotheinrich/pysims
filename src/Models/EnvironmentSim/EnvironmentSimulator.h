#pragma once

#include <Simulator.hpp>
#include <QuantumCHPState.hpp>

class EnvironmentSimulator : public Simulator {
	private:
		uint32_t system_size;

		double int_prob;

		uint32_t env_dim;
		uint32_t env_size;
		
		std::shared_ptr<QuantumCHPState> state;

		bool offset;

		EntropySampler sampler;

	public:
		EnvironmentSimulator(Params& params);

		virtual void init_state(uint32_t) override;
		void one_dimensional_interactions();
		void two_dimensional_interactions();
		virtual void timesteps(uint32_t num_steps) override;

		virtual data_t take_samples() override;

		CLONE(Simulator, EnvironmentSimulator);
};