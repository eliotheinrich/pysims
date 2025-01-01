#include "PostSelectionCliffordSimulator.h"
#include <RandomCliffordSimulator.hpp>

#define DEFAULT_CLIFFORD_TYPE "chp"

using namespace dataframe;
using namespace dataframe::utils;

PostSelectionCliffordSimulator::PostSelectionCliffordSimulator(ExperimentParams &params, uint32_t) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");
	mzr_prob = get<double>(params, "mzr_prob");

	state = std::make_shared<QuantumCHPState>(system_size);
}

void PostSelectionCliffordSimulator::mzr(uint32_t i) {
	state->mzr(i);
}

void PostSelectionCliffordSimulator::timesteps(uint32_t num_steps) {
	for (uint32_t k = 0; k < num_steps; k++) {
		rc_timestep(state, 2, false);
		rc_timestep(state, 2, true);

		for (uint32_t i = 0; i < system_size; i++) {
			if (randf() < mzr_prob) {
				mzr(i);
			}
		}
	}
}

data_t PostSelectionCliffordSimulator::take_samples() {
	data_t samples;
	sampler.add_samples(samples, state);
	return samples;
}

