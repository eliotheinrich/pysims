#ifndef QASIM_H
#define QASIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "CliffordState.hpp"

inline static void qa_layer(std::shared_ptr<CliffordState> state, bool offset, bool gate_type) {
	uint system_size = state->system_size();
	for (uint i = 0; i < system_size/2; i++) {
		uint qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

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

		virtual void init_state() override;
		
		virtual float entropy(const std::vector<uint> &qubits, uint index) const override { return state->entropy(qubits); }

		virtual void timesteps(uint num_steps) override;

		CLONE(Simulator, QuantumAutomatonSimulator)
};

#endif