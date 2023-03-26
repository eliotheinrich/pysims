#include "GraphCliffordSimulator.h"
#include "QuantumGraphState.h"
#include <iostream>

static GSCircuitType parse_gscircuit_type(std::string s) {
	if (s == "random_clifford") return GSCircuitType::GSRandomClifford;
	else if (s == "quantum_automaton") return GSCircuitType::GSQuantumAutomaton;
	else {
		std::cout << "Invalid circuit type: " << s << std::endl;
		assert(false);
	}
}

GraphCliffordSimulator::GraphCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.get<float>("mzr_prob");

	circuit_type = parse_gscircuit_type(params.get<std::string>("circuit", DEFAULT_CIRCUIT_TYPE));

	gate_width = params.get<int>("gate_width");

	initial_offset = false;
}

void GraphCliffordSimulator::init_state() {
	state = std::unique_ptr<QuantumGraphState>(new QuantumGraphState(system_size));
	if (circuit_type == GSCircuitType::GSQuantumAutomaton) { // quantum automaton circuit must be polarized
		for (uint i = 0; i < system_size; i++) state->h_gate(i);
	}
}

void GraphCliffordSimulator::mzr(uint q) {
	state->mzr(q);
	if (circuit_type == GSCircuitType::GSQuantumAutomaton) state->h_gate(q);
}

void GraphCliffordSimulator::qa_timestep(bool offset, bool gate_type) {
	for (uint i = 0; i < system_size/2; i++) {
		uint qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (state->rand() % 2 == 0) std::swap(qubit1, qubit2);

		if (gate_type) state->cz_gate(qubit1, qubit2);
		else state->cx_gate(qubit1, qubit2);
	}
}

void GraphCliffordSimulator::qa_timesteps(uint num_steps) {
	assert(system_size % 2 == 0);

	for (uint i = 0; i < num_steps; i++) {
		qa_timestep(false, false); // no offset, cx
		qa_timestep(false, true);  // no offset, cz
		qa_timestep(true, false);  // offset,    cx
		qa_timestep(true, true);   // offset,    cz

		for (uint j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
				mzr(j);
			}
		}
	}
}

void GraphCliffordSimulator::rc_timesteps(uint num_steps) {
	uint num_qubits = system_size;
	assert(num_qubits % gate_width == 0);
	assert(gate_width % 2 == 0); // So that offset is a whole number

	uint num_gates = num_qubits / gate_width;
	bool offset_layer = initial_offset;

	std::vector<uint> qubits(gate_width);
	std::iota(qubits.begin(), qubits.end(), 0);
	for (uint i = 0; i < num_steps; i++) {
		for (uint j = 0; j < num_gates; j++) {
			uint offset = offset_layer ? gate_width*j : gate_width*j + gate_width/2;

			std::vector<uint> offset_qubits(qubits);
			std::transform(offset_qubits.begin(), offset_qubits.end(), 
						offset_qubits.begin(), [num_qubits, offset](uint x) { return (x + offset) % num_qubits; } );
			state->random_clifford(offset_qubits);
		}

		for (uint j = 0; j < system_size; j++) {
			if (state->randf() < mzr_prob) {
				mzr(j);
			}
		}

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

void GraphCliffordSimulator::timesteps(uint num_steps) {
	switch (circuit_type) {
		case (GSCircuitType::GSRandomClifford): rc_timesteps(num_steps); break;
		case (GSCircuitType::GSQuantumAutomaton): qa_timesteps(num_steps); break;
	}
}

float GraphCliffordSimulator::dist(int i, int j) const {
	uint d = std::abs(i - j);
	if (d > system_size/2) return (system_size - d);
	else return d;
}


std::pair<float, float> GraphCliffordSimulator::avg_max_dist() const {
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

	if (n == 0) return std::pair(0., 0.);

	return std::pair(p1/n, p2/system_size);
}

std::map<std::string, Sample> GraphCliffordSimulator::take_samples() {
	std::map<std::string, Sample> data;
	
	auto degree_counts = state->graph.compute_degree_counts(); 
	for (uint i = 0; i < system_size; i++) data.emplace("deg_" + std::to_string(i), degree_counts[i]);
	auto p = avg_max_dist();
	data.emplace("avg_dist", p.first);
	data.emplace("max_dist", p.second);
	data.emplace("global_clustering_coefficient", state->graph.global_clustering_coefficient());
	data.emplace("average_cluster_size", state->graph.average_component_size());
	data.emplace("max_cluster_size", state->graph.max_component_size());

	return data;
}