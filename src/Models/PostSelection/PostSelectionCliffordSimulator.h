#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

class PostSelectionCliffordSimulator : public dataframe::Simulator {
	private:
		std::shared_ptr<QuantumCHPState> state;

		uint32_t system_size;
		double mzr_prob;

		EntropySampler sampler;


		void mzr(uint32_t i);

	public:
		PostSelectionCliffordSimulator(dataframe::Params &params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::data_t take_samples() override;
};
