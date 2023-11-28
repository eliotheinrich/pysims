#include "QuantumAutomatonSimulator.h"
#include <QuantumCHPState.hpp>
#include <QuantumGraphState.h>
#include <iostream>
#include <assert.h>

#define DEFAULT_CLIFFORD_TYPE "chp"
#define DEFAULT_SAMPLE_SURFACE false

QuantumAutomatonSimulator::QuantumAutomatonSimulator(Params &params) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");
	clifford_type = parse_clifford_type(get<std::string>(params, "clifford_type", DEFAULT_CLIFFORD_TYPE));
	mzr_prob = get<double>(params, "mzr_prob");
	system_size = get<int>(params, "system_size");
	sample_surface = get<int>(params, "sample_surface", DEFAULT_SAMPLE_SURFACE);

	vsample_idx = 0;
}

void QuantumAutomatonSimulator::init_state(uint32_t) {
	switch (clifford_type) {
		case CliffordType::CHP : state = std::make_shared<QuantumCHPState>(system_size); break;
		case CliffordType::GraphSim : state = std::make_shared<QuantumGraphState>(system_size); break;
	}

	// Initially polarize in x-direction
	for (uint32_t i = 0; i < system_size; i++) state->h_gate(i);
}

void QuantumAutomatonSimulator::timesteps(uint32_t num_steps) {
	assert(system_size % 2 == 0);

	for (uint32_t i = 0; i < num_steps; i++) {
		qa_timestep(state);

		for (uint32_t j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
				state->mzr(j);
				state->h_gate(j);
			}
		}
	}
}

data_t QuantumAutomatonSimulator::take_samples() {
	data_t samples;
	sampler.add_samples(samples, state);
	return samples;
}