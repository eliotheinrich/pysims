#include "RandomCliffordSimulator.h"
#include <QuantumCHPState.hpp>
#include <QuantumGraphState.h>
#include <iostream>

#define DEFAULT_GATE_WIDTH 2

#define DEFAULT_CLIFFORD_SIMULATOR "chp"

#define DEFAULT_PERIODIC_BC true

#define DEFAULT_SAMPLE_SPARSITY false

RandomCliffordSimulator::RandomCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = get<double>(params, "mzr_prob");
	gate_width = get<int>(params, "gate_width", DEFAULT_GATE_WIDTH);

	simulator_type = get<std::string>(params, "simulator_type", DEFAULT_CLIFFORD_SIMULATOR);

	initial_offset = false;
	periodic_bc = get<int>(params, "periodic_bc", DEFAULT_PERIODIC_BC);

	sample_sparsity = get<int>(params, "sample_sparsity", DEFAULT_SAMPLE_SPARSITY);	
}

void RandomCliffordSimulator::init_state(uint32_t) {
	if (simulator_type == "chp")
		state = std::make_shared<QuantumCHPState<Tableau>>(system_size);
	else if (simulator_type == "chp_sparse")
		state = std::make_shared<QuantumCHPState<SparseTableau>>(system_size);
	else if (simulator_type == "graph")
		state = std::make_shared<QuantumGraphState>(system_size);
}

void RandomCliffordSimulator::timesteps(uint32_t num_steps) {
	assert(system_size % gate_width == 0);
	assert(gate_width % 2 == 0); // So that offset is a whole number

	bool offset_layer = initial_offset;

	for (uint32_t i = 0; i < num_steps; i++) {
		rc_timestep(state, gate_width, offset_layer, periodic_bc);

		// Apply measurements
		for (uint32_t j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
				take_avalanche_samples();
				state->mzr(j);
				take_avalanche_samples();
			}
		}

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

data_t RandomCliffordSimulator::take_samples() {
	data_t samples = EntropySimulator::take_samples();

	if (sample_sparsity)
		samples.emplace("sparsity", state->sparsity());

	return samples;
}