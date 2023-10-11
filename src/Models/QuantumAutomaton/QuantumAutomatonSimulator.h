#pragma once

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "CliffordState.hpp"

inline static void qa_layer(std::shared_ptr<CliffordState> state, bool offset, bool gate_type) {
	uint32_t system_size = state->system_size();
	for (uint32_t i = 0; i < system_size/2; i++) {
		uint32_t qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint32_t qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (state->rand() % 2 == 0) std::swap(qubit1, qubit2);

		if (gate_type) state->cz_gate(qubit1, qubit2);
		else state->cx_gate(qubit1, qubit2);
	}
}

inline static void qa_timestep(std::shared_ptr<CliffordState> state) {
	qa_layer(state, false, false); // no offset, cx
	qa_layer(state, false, true);  // no offset, cz
	qa_layer(state, true, false);  // offset,    cx
	qa_layer(state, true, true);   // offset,    cz
}

class QuantumAutomatonSimulator : public EntropySimulator {
	private:
		std::shared_ptr<CliffordState> state;
		CliffordType clifford_type;
		float mzr_prob;

		bool sample_surface;

		int vsample_idx;

	public:
		QuantumAutomatonSimulator(Params &params);

		virtual void init_state(uint32_t) override;
		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits); }
		virtual void timesteps(uint32_t num_steps) override;

		CLONE(Simulator, QuantumAutomatonSimulator)
};