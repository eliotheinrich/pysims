#include "QuantumAutomatonSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"
#include <iostream>
#include <assert.h>

QuantumAutomatonSimulator::QuantumAutomatonSimulator(Params &params) : EntropySimulator(params) {
	clifford_type = parse_clifford_type(params.get<std::string>("clifford_type", DEFAULT_CLIFFORD_TYPE));
	mzr_prob = params.get<float>("mzr_prob");
	system_size = params.get<int>("system_size");
	sample_surface = params.get<int>("sample_surface", DEFAULT_SAMPLE_SURFACE);

	vsample_idx = 0;
}

void QuantumAutomatonSimulator::init_state() {
	switch (clifford_type) {
		case CliffordType::CHP : state = std::unique_ptr<CliffordState>(new QuantumCHPState(system_size)); break;
		case CliffordType::GraphSim : state = std::unique_ptr<CliffordState>(new QuantumGraphState(system_size)); break;
	}

	// Initially polarize in x-direction
	for (uint i = 0; i < system_size; i++) state->h_gate(i);
}

void QuantumAutomatonSimulator::timestep(bool offset, bool gate_type) {
	for (uint i = 0; i < system_size/2; i++) {
		uint qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (state->rand() % 2 == 0) std::swap(qubit1, qubit2);

		if (gate_type) state->cz_gate(qubit1, qubit2);
		else state->cx_gate(qubit1, qubit2);
	}
}

void QuantumAutomatonSimulator::timesteps(uint num_steps) {
	assert(system_size % 2 == 0);

	for (uint i = 0; i < num_steps; i++) {
		timestep(false, false); // no offset, cx
		timestep(false, true);  // no offset, cz
		timestep(true, false);  // offset,    cx
		timestep(true, true);   // offset,    cz

		for (uint j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
				state->mzr(j);
				state->h_gate(j);
			}
		}
	}
}