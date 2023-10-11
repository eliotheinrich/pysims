#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumCHPState.hpp>

class PostSelectionCliffordSimulator : public EntropySimulator {
	private:
		std::shared_ptr<QuantumCHPState<Tableau>> state;

		float mzr_prob;


		void mzr(uint32_t i);

	public:
		PostSelectionCliffordSimulator(Params &params);

		virtual void init_state(uint32_t) override {
			state = std::shared_ptr<QuantumCHPState<Tableau>>(new QuantumCHPState(system_size));
		}

		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits); }
		virtual void timesteps(uint32_t num_steps) override;

		CLONE(Simulator, PostSelectionCliffordSimulator)
};