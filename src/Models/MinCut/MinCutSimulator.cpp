#include "MinCutSimulator.h"

using namespace dataframe;
using namespace dataframe::utils;

double GraphEntropyState::entanglement(const QubitSupport &support, uint32_t index) {
  Qubits sites = to_qubits(support);
	if (sites.size() % 2) {
		throw std::invalid_argument("Number of sites must be even in MinCutSimulator.");
	}

	uint32_t num_vertices = state.num_vertices;
	uint32_t system_size = 2*state.num_vertices;

	uint32_t tsteps = num_vertices/(system_size/2) - 1;
	uint32_t d = tsteps*system_size/2;

	std::vector<uint32_t> subsystem_a;
	for (auto q : sites) {
		if (q % 2 == 0) subsystem_a.push_back(q/2 + d);
	}

	uint32_t q1 = subsystem_a.front();
	uint32_t q2 = subsystem_a.back();

	std::vector<uint32_t> subsystem_b;
	for (uint32_t i = d; i < d + system_size/2; i++) {
		if ((i < q1) || (i > q2)) {
			subsystem_b.push_back(i);
		}
	}

	return static_cast<double>(state.max_flow(subsystem_a, subsystem_b));
}


MinCutSimulator::MinCutSimulator(ExperimentParams &params, uint32_t) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");
	mzr_prob = get<double>(params, "mzr_prob");

	state = std::make_shared<GraphEntropyState>(system_size/2);
}

std::string MinCutSimulator::to_string() const {
	return state->state.to_string();
}

void MinCutSimulator::timesteps(uint32_t num_steps) {
	for (uint32_t t = 0; t < num_steps; t++) {
		uint32_t num_vertices = state->state.num_vertices;
		uint32_t num_new_vertices = system_size/2;
		for (uint32_t i = 0; i < num_new_vertices; i++) {
			state->state.add_vertex();
		}

		for (uint32_t i = 0; i < num_new_vertices; i++) {
			uint32_t v1 = num_vertices + i;
			uint32_t col = i;
			uint32_t row = (v1 - i) / num_new_vertices;

			uint32_t next_col = offset ? mod(col + 1, num_new_vertices) : mod(col - 1, num_new_vertices);
			uint32_t v2 = (row - 1)*num_new_vertices + col;
			uint32_t v3 = (row - 1)*num_new_vertices + next_col;

			if (randf() < 1. - mzr_prob) {
				state->state.add_edge(v1, v2);
			}
			if (randf() < 1. - mzr_prob) {
				state->state.add_edge(v1, v3);
			}
		}

	}
	
	if (num_steps % 2 == 1) {
		offset = !offset;
	}
}


SampleMap MinCutSimulator::take_samples() {
	SampleMap samples;
	sampler.add_samples(samples, state);
	return samples;
}
