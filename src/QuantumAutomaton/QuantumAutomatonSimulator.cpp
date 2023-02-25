#include "QuantumAutomatonSimulator.h"
#include "QuantumCHPState.h"
#include <iostream>
#include <assert.h>

QuantumAutomatonSimulator::QuantumAutomatonSimulator(uint system_size, float mzr_prob, CliffordType simulator_type) : mzr_prob(mzr_prob) {
	switch (simulator_type) {
		case CHP : state = new QuantumCHPState(system_size); break;
		case GraphSim : std::cout << "Graph state not implemented yet.\n"; break;
	}

	// Initially polarize in x-direction
	for (uint i = 0; i < system_size; i++)
		state->h_gate(i);
}

QuantumAutomatonSimulator::~QuantumAutomatonSimulator() {
	delete state;
}

void QuantumAutomatonSimulator::timestep(bool offset, bool gate_type) {
	uint num_qubits = state->system_size();
	for (uint i = 0; i < num_qubits/2; i++) {
		uint qubit1 = offset ? (2*i + 1) % num_qubits : 2*i;
		uint qubit2 = offset ? (2*i + 2) % num_qubits : (2*i + 1) % num_qubits;

		if (state->rand() % 2 == 0) 
			std::swap(qubit1, qubit2);

		if (gate_type)
			state->cz_gate(qubit1, qubit2);
		else
			state->cx_gate(qubit1, qubit2);
	}
}

void QuantumAutomatonSimulator::timesteps(uint num_steps) {
	assert(state->system_size() % 2 == 0);

	for (uint i = 0; i < num_steps; i++) {
		timestep(false, false); // no offset, cx
		timestep(false, true);  // no offset, cz
		timestep(true, false);  // offset,    cx
		timestep(true, true);   // offset,    cz

		for (uint j = 0; j < state->system_size(); j++) {
			if (state->randf() < mzr_prob) {
				state->mzr(j);
				state->h_gate(j);
			}
		}
	}
}
