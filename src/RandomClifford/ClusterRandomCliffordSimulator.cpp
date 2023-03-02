#include "ClusterRandomCliffordSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"
#include <iostream>

ClusterRandomCliffordSimulator::ClusterRandomCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.getf("mzr_prob");
	gate_width = params.geti("gate_width");
	initial_offset = false;
}

void ClusterRandomCliffordSimulator::init_state() {
	state = new QuantumGraphState(system_size);
}

ClusterRandomCliffordSimulator::~ClusterRandomCliffordSimulator() {
	delete state;
}

void ClusterRandomCliffordSimulator::timesteps(uint num_steps) {
	uint num_qubits = state->system_size();
	assert(num_qubits % gate_width == 0);
	assert(gate_width % 2 == 0); // So that offset is a whole number

	uint num_gates = num_qubits / gate_width;
	bool offset_layer = initial_offset;

	std::vector<uint> qubits(gate_width);
	std::iota(qubits.begin(), qubits.end(), 0);
	for (uint i = 0; i < num_steps; i++) {
		for (uint j = 0; j < num_gates; j++) {
			uint offset = offset_layer ? gate_width*j : gate_width*j + gate_width/2;

			std::vector<uint> offset_qubits(qubits);
			std::transform(offset_qubits.begin(), offset_qubits.end(), 
						   offset_qubits.begin(), [num_qubits, offset](uint x) { return (x + offset) % num_qubits; } );
			state->random_clifford(offset_qubits);
		}

		// Apply measurements
		for (uint j = 0; j < num_qubits; j++) {
			if (state->randf() < mzr_prob) {
				std::set<uint> cluster = state->graph->component(j);
				for (auto k : cluster) state->mzr(k);
			}
		}

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}
