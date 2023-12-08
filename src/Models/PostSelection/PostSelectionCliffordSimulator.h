#pragma once

#include <Simulator.hpp>
#include <QuantumCHPState.hpp>

class PostSelectionCliffordSimulator : public Simulator {
	private:
		std::shared_ptr<QuantumCHPState> state;

		uint32_t system_size;
		double mzr_prob;

		EntropySampler sampler;


		void mzr(uint32_t i);

	public:
		PostSelectionCliffordSimulator(Params &params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;

		virtual data_t take_samples() override;
};