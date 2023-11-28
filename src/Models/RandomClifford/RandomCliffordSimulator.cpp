#include "RandomCliffordSimulator.h"
#include <QuantumCHPState.hpp>
#include <QuantumGraphState.h>

#define DEFAULT_GATE_WIDTH 2

#define DEFAULT_CLIFFORD_SIMULATOR "chp"

#define DEFAULT_PBC true

#define DEFAULT_SAMPLE_SPARSITY false

RandomCliffordSimulator::RandomCliffordSimulator(Params &params) : Simulator(params), entropy_sampler(params), interface_sampler(params) {
	system_size = get<int>(params, "system_size");

	mzr_prob = get<double>(params, "mzr_prob");
	gate_width = get<int>(params, "gate_width", DEFAULT_GATE_WIDTH);

	simulator_type = get<std::string>(params, "simulator_type", DEFAULT_CLIFFORD_SIMULATOR);

	sample_avalanche_sizes = get<int>(params, "sample_avalanche_sizes", false);

	initial_offset = false;
	pbc = get<int>(params, "pbc", DEFAULT_PBC);

	sample_sparsity = get<int>(params, "sample_sparsity", DEFAULT_SAMPLE_SPARSITY);	

	seed = get<int>(params, "seed", -1);

	start_sampling = false;
}

void RandomCliffordSimulator::init_state(uint32_t) {
	if (simulator_type == "chp")
		state = std::make_shared<QuantumCHPState<Tableau>>(system_size, seed);
	else if (simulator_type == "chp_sparse")
		state = std::make_shared<QuantumCHPState<SparseTableau>>(system_size, seed);
	else if (simulator_type == "graph")
		state = std::make_shared<QuantumGraphState>(system_size, seed);
}

std::shared_ptr<Simulator> RandomCliffordSimulator::deserialize(Params &params, const std::string &data) {
	std::shared_ptr<RandomCliffordSimulator> sim(new RandomCliffordSimulator(params));
	sim->state = std::make_shared<QuantumCHPState<Tableau>>(data);
	return sim;
}

void RandomCliffordSimulator::timesteps(uint32_t num_steps) {
	if (system_size % gate_width != 0)
		throw std::invalid_argument("Invalid gate width. Must divide system size.");
	if (gate_width % 2 != 0)
		throw std::invalid_argument("Gate width must be even.");

	bool offset_layer = initial_offset;

	for (uint32_t i = 0; i < num_steps; i++) {
		rc_timestep(state, gate_width, offset_layer, pbc);

		// Apply measurements
		for (uint32_t j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
				if (sample_avalanche_sizes && start_sampling) {
					std::vector<int> surface1 = state->get_entropy_surface<int>(2);
					state->mzr(j);
					std::vector<int> surface2 = state->get_entropy_surface<int>(2);

					int s = 0.0;
					for (uint32_t i = 0; i < system_size; i++)
						s += std::abs(surface1[i] - surface2[i]);

					interface_sampler.record_size(s);
				} else {
					state->mzr(j);
				}
			}
		}

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

data_t RandomCliffordSimulator::take_samples() {
	data_t samples;

	entropy_sampler.add_samples(samples, state);

	std::vector<int> surface = state->get_entropy_surface<int>(2);
	interface_sampler.add_samples(samples, surface);

	if (sample_sparsity)
		samples.emplace("sparsity", state->sparsity());

	return samples;
}