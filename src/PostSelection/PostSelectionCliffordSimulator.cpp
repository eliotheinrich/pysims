#include "PostSelectionCliffordSimulator.h"
#include "QuantumCHPState.h"
#include "RandomCliffordSimulator.h"
#include <iostream>

#define DEFAULT_CLIFFORD_TYPE "chp"

PostSelectionCliffordSimulator::PostSelectionCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.get<float>("mzr_prob");
}

void PostSelectionCliffordSimulator::init_state() {
	state = std::shared_ptr<QuantumCHPState>(new QuantumCHPState(system_size));
}

void PostSelectionCliffordSimulator::mzr(uint i) {
	state->mzr(i);
}

void PostSelectionCliffordSimulator::timesteps(uint num_steps) {
	for (uint k = 0; k < num_steps; k++) {
		rc_timestep(state, 2, false);
		rc_timestep(state, 2, true);

		for (uint i = 0; i < system_size; i++) {
			if (randf() < mzr_prob)
				mzr(i);
		}
	}
}


