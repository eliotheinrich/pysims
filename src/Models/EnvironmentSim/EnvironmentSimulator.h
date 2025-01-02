#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

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
		EnvironmentSimulator(dataframe::ExperimentParams& params, uint32_t);

		void one_dimensional_interactions();
		void two_dimensional_interactions();
		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::SampleMap take_samples() override;
};
