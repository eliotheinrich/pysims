#include "GraphCliffordSimulator.h"
#include "QuantumGraphState.h"
#include "RandomCliffordSimulator.h"
#include "QuantumAutomatonSimulator.h"
#include <iostream>

#define DEFAULT_CIRCUIT_TYPE "random_clifford"

static GraphSimCircuitType parse_graphsim_circuit_type(std::string s) {
	if (s == "random_clifford") return GraphSimCircuitType::GraphSimRandomClifford;
	else if (s == "quantum_automaton") return GraphSimCircuitType::GraphSimQuantumAutomaton;
	else if (s == "unitary") return GraphSimCircuitType::GraphSimUnitary;
	else {
		std::cout << "Invalid circuit type: " << s << std::endl;
		assert(false);
	}
}

GraphCliffordSimulator::GraphCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.get<float>("mzr_prob");

	circuit_type = parse_graphsim_circuit_type(params.get<std::string>("circuit", DEFAULT_CIRCUIT_TYPE));

	gate_width = params.get<int>("gate_width");

	initial_offset = false;
}

void GraphCliffordSimulator::init_state() {
	state = std::shared_ptr<QuantumGraphState>(new QuantumGraphState(system_size));

	if (circuit_type == GraphSimCircuitType::GraphSimQuantumAutomaton) // quantum automaton circuit must be polarized
		for (uint i = 0; i < system_size; i++) state->h_gate(i);
}

void GraphCliffordSimulator::mzr(uint q) {
	state->mzr(q);
	if (circuit_type == GraphSimCircuitType::GraphSimQuantumAutomaton) state->h_gate(q);
}

void GraphCliffordSimulator::qa_timesteps(uint num_steps) {
	assert(system_size % 2 == 0);

	for (uint i = 0; i < num_steps; i++) {
		qa_timestep(state);

		for (uint j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob)
				mzr(j);
		}
	}
}

void GraphCliffordSimulator::rc_timesteps(uint num_steps) {
	assert(system_size % gate_width == 0);
	assert(gate_width % 2 == 0); // So that offset is a whole number

	bool offset_layer = initial_offset;

	for (uint i = 0; i < num_steps; i++) {
		rc_timestep(state, gate_width, offset_layer);

		for (uint j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob)
				mzr(j);
		}

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

void GraphCliffordSimulator::unitary_timesteps(uint num_steps) {
	assert(system_size % gate_width == 0);
	assert(gate_width % 2 == 0); // So that offset is a whole number

	bool offset_layer = initial_offset;

	for (uint i = 0; i < num_steps; i++) {
		rc_timestep(state, gate_width, offset_layer);

		for (uint j = 0; j < system_size; j++) {
			auto neighbors = state->graph.neighbors(j);
			for (auto k : neighbors) {
				if (randf() < mzr_prob)
					state->toggle_edge_gate(j, k);
			}
		}

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

void GraphCliffordSimulator::timesteps(uint num_steps) {
	switch (circuit_type) {
		case (GraphSimCircuitType::GraphSimRandomClifford): rc_timesteps(num_steps); break;
		case (GraphSimCircuitType::GraphSimQuantumAutomaton): qa_timesteps(num_steps); break;
		case (GraphSimCircuitType::GraphSimUnitary): unitary_timesteps(num_steps); break;
	}
}

uint GraphCliffordSimulator::dist(int i, int j) const {
	uint d = std::abs(i - j);
	if (d > system_size/2) 
		return (system_size - d);
	else 
		return d;
}

void GraphCliffordSimulator::add_distance_distribution(std::map<std::string, Sample> &samples) const {
	uint max_dist = system_size/2;
	std::vector<uint> distribution(max_dist, 0.);
	for (uint i = 0; i < system_size; i++) {
		for (auto const j : state->graph.neighbors(i))
			distribution[dist(i, j)]++;
	}

	for (uint i = 0; i < max_dist; i++) 
		samples.emplace("dist_" + std::to_string(i), distribution[i]);
}


void GraphCliffordSimulator::add_avg_max_dist(std::map<std::string, Sample> &samples) const {
	double p1 = 0.;
	double p2 = 0.;
	uint n = 0;
	for (uint i = 0; i < system_size; i++) {
		float max_d = 0.;
		for (auto j : state->graph.neighbors(i)) {
			float d = dist(i, j);
			p1 += d;
			if (d > max_d) max_d = d;

			n++;
		}

		p2 += max_d;
	}

	if (n == 0) {
		samples.emplace("avg_dist", 0.);
		samples.emplace("max_dist", 0.);
	} else {
		samples.emplace("avg_dist", p1/n);
		samples.emplace("max_dist", p2/system_size);
	}
}

void GraphCliffordSimulator::add_degree_distribution(std::map<std::string, Sample> &samples) const {
	auto degree_counts = state->graph.compute_degree_counts();
	for (uint i = 0; i < system_size; i++) 
		samples.emplace("deg_" + std::to_string(i), degree_counts[i]);
}

std::map<std::string, Sample> GraphCliffordSimulator::take_samples() {
	std::map<std::string, Sample> samples;
	
	add_avg_max_dist(samples);

	add_distance_distribution(samples);
	add_degree_distribution(samples);

	samples.emplace("global_clustering_coefficient", state->graph.global_clustering_coefficient());
	samples.emplace("average_cluster_size", state->graph.average_component_size());
	samples.emplace("max_cluster_size", state->graph.max_component_size());

	// Add EntropySimulator samples
	auto parent_samples = EntropySimulator::take_samples();
	samples.insert(parent_samples.begin(), parent_samples.end());

	return samples;
}