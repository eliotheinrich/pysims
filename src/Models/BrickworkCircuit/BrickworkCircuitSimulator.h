#pragma once

#include <Simulator.hpp>
#include <QuantumState.h>
#include <Samplers.h>

class BrickworkCircuitSimulator : public Simulator {
	private:
		uint32_t system_size;

		double mzr_prob;
		int gate_type;

		bool offset;

		EntropySampler sampler;

		void mzr(uint32_t q);

	public:
		std::shared_ptr<Statevector> state;

		BrickworkCircuitSimulator(dataframe::ExperimentParams &params, uint32_t num_threads);

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::SampleMap take_samples() override;
};
