#include "QuantumAutomatonSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"
#include <iostream>
#include <assert.h>

QuantumAutomatonSimulator::QuantumAutomatonSimulator(Params &params) : EntropySimulator(params) {
	clifford_type = parse_clifford_type(params.gets("clifford_type", DEFAULT_CLIFFORD_TYPE));
	mzr_prob = params.getf("mzr_prob");
	system_size = params.geti("system_size");
}

void QuantumAutomatonSimulator::init_state() {
	switch (clifford_type) {
		case CHP : state = new QuantumCHPState(system_size); break;
		case GraphSim : state = new QuantumGraphState(system_size); break;
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

std::map<std::string, Sample> QuantumAutomatonSimulator::take_samples() const {
	std::map<std::string, Sample> sample;
	sample.emplace("entropy", spatially_averaged_entropy());
	return sample;
}