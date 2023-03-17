#include "ClusterCliffordSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"
#include <iostream>

ClusterCliffordSimulator::ClusterCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.getf("mzr_prob");
	x = std::log(mzr_prob/(1. - mzr_prob));

	cluster_threshold = params.getf("cluster_threshold", DEFAULT_CLUSTER_THRESHOLD);

	circuit_type = params.geti("circuit", DEFAULT_CIRCUIT);
	soc_type = params.geti("soc_type");

	gate_width = params.geti("gate_width");

	initial_offset = false;
}

void ClusterCliffordSimulator::init_state() {
	avalanche_size = Sample();
	state = std::unique_ptr<QuantumGraphState>(new QuantumGraphState(system_size));
	if (circuit_type == 1) {
		for (uint i = 0; i < system_size; i++) state->h_gate(i);
	}
}

void ClusterCliffordSimulator::mzr(uint q) {
	state->mzr(q);
	if (circuit_type == 1) state->h_gate(q);
}

void ClusterCliffordSimulator::random_measure() {
	for (uint j = 0; j < system_size; j++) {
		if (state->randf() < mzr_prob) {
			mzr(j);
		}
	}
}

void ClusterCliffordSimulator::cluster_measure() {
	if (randf() > mzr_prob) return;

	uint q = rand() % system_size;
	std::set<uint> cluster = state->graph.component(q);
	for (auto k : cluster) {
		mzr(k);
	}
	avalanche_size = avalanche_size.combine(Sample(cluster.size()));
}

void ClusterCliffordSimulator::p_adjust() {
	random_measure();

	float dx = 0.05;
	if (float(state->graph.max_component_size())/system_size > cluster_threshold) x += dx;
	else x -= dx;

	mzr_prob = 1./(1. + std::exp(-x));
}


void ClusterCliffordSimulator::mzr_feedback() {
	// Apply measurements
	if (soc_type == 0) { // No feedback
		random_measure();
	} else if (soc_type == 1) { // Measure nodes in cluster
		cluster_measure();
	} else if (soc_type == 2) { // Adjust p based on max cluster size
		p_adjust();
	} 
}


void ClusterCliffordSimulator::qa_timestep(bool offset, bool gate_type) {
	for (uint i = 0; i < system_size/2; i++) {
		uint qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (state->rand() % 2 == 0) std::swap(qubit1, qubit2);

		if (gate_type) state->cz_gate(qubit1, qubit2);
		else state->cx_gate(qubit1, qubit2);
	}
}

void ClusterCliffordSimulator::qa_timesteps(uint num_steps) {
	assert(system_size % 2 == 0);

	for (uint i = 0; i < num_steps; i++) {
		qa_timestep(false, false); // no offset, cx
		qa_timestep(false, true);  // no offset, cz
		qa_timestep(true, false);  // offset,    cx
		qa_timestep(true, true);   // offset,    cz

		mzr_feedback();
	}
}

void ClusterCliffordSimulator::rc_timesteps(uint num_steps) {
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

		mzr_feedback();

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

void ClusterCliffordSimulator::timesteps(uint num_steps) {
	if (circuit_type == 0) { // Random clifford gates
		rc_timesteps(num_steps);
	} else if (circuit_type == 1) { // Quantum automaton
		qa_timesteps(num_steps);
	} else {
		std::cout << "Invalid circuit type " << circuit_type << std::endl;
		assert(false);
	}
}

std::map<std::string, Sample> ClusterCliffordSimulator::take_samples() {
	std::map<std::string, Sample> data;
	for (auto const &[key, val] : EntropySimulator::take_samples()) data.emplace(key, val);
	if (soc_type == 1) {
		data.emplace("avalanche_size", avalanche_size);
		avalanche_size = Sample();
	} else if (soc_type == 2) {
		data.emplace("mzr_prob_f", mzr_prob);
	}
	data.emplace("max_cluster_size", state->graph.max_component_size());
	return data;
}