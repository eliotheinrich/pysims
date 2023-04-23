#include "QuantumAutomatonSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"
#include <iostream>
#include <assert.h>

#define DEFAULT_CLIFFORD_TYPE "chp"
#define DEFAULT_SAMPLE_SURFACE false

QuantumAutomatonSimulator::QuantumAutomatonSimulator(Params &params) : EntropySimulator(params) {
	clifford_type = parse_clifford_type(params.get<std::string>("clifford_type", DEFAULT_CLIFFORD_TYPE));
	mzr_prob = params.get<float>("mzr_prob");
	system_size = params.get<int>("system_size");
	sample_surface = params.get<int>("sample_surface", DEFAULT_SAMPLE_SURFACE);

	vsample_idx = 0;
}

void QuantumAutomatonSimulator::init_state() {
	switch (clifford_type) {
		case CliffordType::CHP : state = std::shared_ptr<CliffordState>(new QuantumCHPState(system_size)); break;
		case CliffordType::GraphSim : state = std::shared_ptr<CliffordState>(new QuantumGraphState(system_size)); break;
	}

	// Initially polarize in x-direction
	for (uint i = 0; i < system_size; i++) state->h_gate(i);
}

void QuantumAutomatonSimulator::timesteps(uint num_steps) {
	assert(system_size % 2 == 0);

	for (uint i = 0; i < num_steps; i++) {
		qa_timestep(state);

		for (uint j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
				state->mzr(j);
				state->h_gate(j);
			}
		}
	}
}