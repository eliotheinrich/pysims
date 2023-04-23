#include "RandomCliffordSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"
#include <iostream>

#define DEFAULT_CLIFFORD_TYPE "chp"
#define DEFAULT_GATE_WIDTH 2

RandomCliffordSimulator::RandomCliffordSimulator(Params &params) : EntropySimulator(params) {
	clifford_type = parse_clifford_type(params.get<std::string>("clifford_type", DEFAULT_CLIFFORD_TYPE));
	mzr_prob = params.get<float>("mzr_prob");
	gate_width = params.get<int>("gate_width", DEFAULT_GATE_WIDTH);

	initial_offset = false;
}

void RandomCliffordSimulator::init_state() {
	switch (clifford_type) {
		case CliffordType::CHP : state = std::shared_ptr<CliffordState>(new QuantumCHPState(system_size)); break;
		case CliffordType::GraphSim : state = std::shared_ptr<CliffordState>(new QuantumGraphState(system_size)); break;
	}
}

void RandomCliffordSimulator::timesteps(uint num_steps) {
	assert(system_size % gate_width == 0);
	assert(gate_width % 2 == 0); // So that offset is a whole number

	bool offset_layer = initial_offset;

	for (uint i = 0; i < num_steps; i++) {
		rc_timestep(state, gate_width, offset_layer);

		// Apply measurements
		for (uint j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob)
				state->mzr(j);
		}

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}


