#include "SelfOrganizedRandomCliffordSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"
#include <iostream>

SelfOrganizedRandomCliffordSimulator::SelfOrganizedRandomCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.getf("mzr_prob");
	x = std::log(mzr_prob/(1. - mzr_prob));

	cluster_threshold = params.getf("cluster_threshold", DEFAULT_CLUSTER_THRESHOLD);

	gate_width = params.geti("gate_width");
	initial_offset = false;
	soc_type = params.geti("soc_type");
}

void SelfOrganizedRandomCliffordSimulator::init_state() {
	avalanche_size = Sample();
	state = std::unique_ptr<QuantumGraphState>(new QuantumGraphState(system_size));
}

void SelfOrganizedRandomCliffordSimulator::timesteps(uint num_steps) {
	uint num_qubits = state->system_size();
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

		// Apply measurements
		if (soc_type == 0) {
			random_measure();
		} else if (soc_type == 1) {
			cluster_measure();
		} else if (soc_type == 2) {
			p_adjust();
		} 
		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

void SelfOrganizedRandomCliffordSimulator::random_measure() {
	for (uint j = 0; j < system_size; j++) {
		if (state->randf() < mzr_prob) {
			state->mzr(j);
		}
	}
}

void SelfOrganizedRandomCliffordSimulator::cluster_measure() {
	if (randf() > mzr_prob) return;

	uint q = rand() % system_size;
	std::set<uint> cluster = state->graph.component(q);
	for (auto k : cluster) state->mzr(k);
	avalanche_size = avalanche_size.combine(Sample(cluster.size()));
}

void SelfOrganizedRandomCliffordSimulator::p_adjust() {
	random_measure();

	float dx = 0.05;
	if (float(state->graph.max_component_size())/system_size > cluster_threshold) x += dx;
	else x -= dx;

	mzr_prob = 1./(1. + std::exp(-x));
}

std::map<std::string, Sample> SelfOrganizedRandomCliffordSimulator::take_samples() {
	std::map<std::string, Sample> data;
	for (auto const &[key, val] : EntropySimulator::take_samples()) data.emplace(key, val);
	if (soc_type == 1) {
		data.emplace("avalanche_size", avalanche_size);
		avalanche_size = Sample();
	} else if (soc_type == 2) {
		data.emplace("mzr_prob_f", mzr_prob);
	}
	return data;
}