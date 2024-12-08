#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

class BulkMeasurementSimulator : public Simulator {
	private:
		uint32_t system_size;
		uint32_t L;

		double mzr_prob;

    	uint32_t circuit_depth;

		std::shared_ptr<QuantumCHPState> state;

		EntropySampler sampler;

		std::pair<uint32_t, uint32_t> two_dim_coordinates(uint32_t, uint32_t);
		uint32_t site_index(uint32_t, uint32_t, uint32_t);

	public:
		BulkMeasurementSimulator(dataframe::Params& params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::data_t take_samples() override;
};
