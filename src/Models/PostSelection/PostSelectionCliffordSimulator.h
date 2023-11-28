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
		PostSelectionCliffordSimulator(Params &params);

		virtual void init_state(uint32_t) override {
			state = std::shared_ptr<QuantumCHPState>(new QuantumCHPState(system_size));
		}

		virtual void timesteps(uint32_t num_steps) override;

		virtual data_t take_samples() override;

		CLONE(Simulator, PostSelectionCliffordSimulator)
};