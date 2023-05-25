#ifndef PS_SIM_H
#define PS_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

class PostSelectionCliffordSimulator : public EntropySimulator {
	private:
		std::shared_ptr<QuantumCHPState> state;

		float mzr_prob;


		void mzr(uint i);

	public:
		PostSelectionCliffordSimulator(Params &params);

		virtual void init_state() override;

		virtual float entropy(const std::vector<uint> &qubits) const override { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps) override;

		CLONE(Simulator, PostSelectionCliffordSimulator)
};

#endif