#include "MinCutSimulator.h"
#include <assert.h>

void MinCutSimulator::init_state() {
	state = Graph(system_size/2);
}

MinCutSimulator::MinCutSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.get<float>("mzr_prob");
}

std::string MinCutSimulator::to_string() const {
	return state.to_string();
}

float MinCutSimulator::entropy(std::vector<uint> &qubits) const {
	assert(qubits.size() % 2 == 0);
	uint num_vertices = state.num_vertices;

	uint tsteps = num_vertices/(system_size/2) - 1;
	uint d = tsteps*system_size/2;

	std::vector<uint> subsystem_a;
	for (auto q : qubits)
		if (q % 2 == 0) subsystem_a.push_back(q/2 + d);

	uint q1 = subsystem_a.front();
	uint q2 = subsystem_a.back();

	std::vector<uint> subsystem_b;
	for (uint i = d; i < d + system_size/2; i++) {
		if ((i < q1) || (i > q2)) subsystem_b.push_back(i);
	}

	return state.max_flow(subsystem_a, subsystem_b);
}

void MinCutSimulator::timesteps(uint num_steps) {
	for (uint t = 0; t < num_steps; t++) {
		uint num_vertices = state.num_vertices;
		uint num_new_vertices = system_size/2;
		for (uint i = 0; i < num_new_vertices; i++) state.add_vertex();

		for (uint i = 0; i < num_new_vertices; i++) {
			uint v1 = num_vertices + i;
			uint col = i;
			uint row = (v1 - i) / num_new_vertices;

			uint next_col = offset ? mod(col + 1, num_new_vertices) : mod(col - 1, num_new_vertices);
			uint v2 = (row - 1)*num_new_vertices + col;
			uint v3 = (row - 1)*num_new_vertices + next_col;

			//std::cout << "mzr_prob: " << mzr_prob << std::endl;
			if (randf() < 1. - mzr_prob) state.add_edge(v1, v2);
			if (randf() < 1. - mzr_prob) state.add_edge(v1, v3);
		}

	}
	
	if (num_steps % 2 == 1) offset = !offset;
}
