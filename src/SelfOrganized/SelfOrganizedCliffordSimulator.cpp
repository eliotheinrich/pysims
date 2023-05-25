#include "SelfOrganizedCliffordSimulator.h"
#include "QuantumCHPState.h"
#include "QuantumGraphState.h"

#define DEFAULT_THRESHOLD 0.5
#define DEFAULT_EVOLUTION_TYPE "random_clifford"
#define DEFAULT_DX 0.05
#define DEFAULT_GATE_WIDTH 2


static EvolutionType parse_evolution_type(std::string s) {
	if (s == "random_clifford") return EvolutionType::RandomClifford;
	else if (s == "quantum_automaton") return EvolutionType::QuantumAutomaton;
	else if (s == "graph_operations") return EvolutionType::GraphOperations;
	else {
		std::cout << "Invalid evolution type: " << s << std::endl;
		assert(false);
	}
}

static FeedbackType parse_feedback_type(std::string s) {
	if (s == "no_feedback") return FeedbackType::NoFeedback;
	else if (s == "cluster_threshold") return FeedbackType::ClusterThreshold;
	else if (s == "distance_threshold") return FeedbackType::DistanceThreshold;
	else {
		std::cout << "Invalid feedback type: " << s << std::endl;
		assert(false);
	}
}

SelfOrganizedCliffordSimulator::SelfOrganizedCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.get<float>("mzr_prob");
	x = std::log(mzr_prob/(1. - mzr_prob));

	threshold = params.get<float>("threshold", DEFAULT_THRESHOLD);

	evolution_type = parse_evolution_type(params.get<std::string>("evolution_type", DEFAULT_EVOLUTION_TYPE));
	feedback_type = parse_feedback_type(params.get<std::string>("feedback_type"));
	dx = params.get<float>("dx", DEFAULT_DX);

	gate_width = params.get<int>("gate_width", DEFAULT_GATE_WIDTH);

	avalanche_size = 0;

	initial_offset = false;
}

void SelfOrganizedCliffordSimulator::init_state() {
	state = std::unique_ptr<QuantumGraphState>(new QuantumGraphState(system_size));
	if (evolution_type == EvolutionType::QuantumAutomaton) { // quantum automaton circuit must be polarized
		for (uint i = 0; i < system_size; i++) state->h_gate(i);
	}
}

void SelfOrganizedCliffordSimulator::mzr(uint q) {
	state->mzr(q);
	if (evolution_type == EvolutionType::QuantumAutomaton) state->h_gate(q);
}

uint SelfOrganizedCliffordSimulator::dist(int i, int j) const {
	uint d = std::abs(i - j);
	if (d > system_size/2) 
		return (system_size - d);
	else 
		return d;
}

float SelfOrganizedCliffordSimulator::avg_dist() const {
	float d = 0.;
	uint n = 0;
	for (uint i = 0; i < system_size; i++) {
		for (auto const &j : state->graph.neighbors(i)) {
			d += dist(i, j);
			n++;
		}
	}

	if (n == 0) return 0.;
	else return d/n;
}

float SelfOrganizedCliffordSimulator::max_component_size() const {
	return state->graph.max_component_size();
}

void SelfOrganizedCliffordSimulator::random_measure() {
	for (uint j = 0; j < system_size; j++) {
		if (state->randf() < mzr_prob)
			mzr(j);
	}
}

void SelfOrganizedCliffordSimulator::cluster_threshold() {
	random_measure();

	if (max_component_size()/system_size > threshold) x += dx;
	else x -= dx;

	mzr_prob = 1./(1. + std::exp(-x));
}

void SelfOrganizedCliffordSimulator::distance_threshold() {
	random_measure();

	if (avg_dist()/system_size > threshold) x += dx;
	else x -= dx;

	mzr_prob = 1./(1. + std::exp(-x));
}

void SelfOrganizedCliffordSimulator::mzr_feedback() {
	// Apply measurements
	switch (feedback_type) {
		case (FeedbackType::NoFeedback): random_measure(); break;
		case (FeedbackType::ClusterThreshold): cluster_threshold(); break;
		case (FeedbackType::DistanceThreshold): distance_threshold(); break;
	}
}


void SelfOrganizedCliffordSimulator::qa_timestep(bool offset, bool gate_type) {
	for (uint i = 0; i < system_size/2; i++) {
		uint qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (state->rand() % 2 == 0) std::swap(qubit1, qubit2);

		if (gate_type) state->cz_gate(qubit1, qubit2);
		else state->cx_gate(qubit1, qubit2);
	}
}

void SelfOrganizedCliffordSimulator::qa_timesteps(uint num_steps) {
	assert(system_size % 2 == 0);

	for (uint i = 0; i < num_steps; i++) {
		qa_timestep(false, false); // no offset, cx
		qa_timestep(false, true);  // no offset, cz
		qa_timestep(true, false);  // offset,    cx
		qa_timestep(true, true);   // offset,    cz

		mzr_feedback();
	}
}

void SelfOrganizedCliffordSimulator::rc_timesteps(uint num_steps) {
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

void SelfOrganizedCliffordSimulator::graph_timesteps(uint num_steps) {
	for (uint i = 0; i < system_size; i += 2) {
		uint j = (i + 1) % system_size;
		if (!state->graph.contains_edge(i, j)) {
			state->toggle_edge_gate(i, j);
		} else {
			state->graph.local_complement(i);
			state->graph.local_complement(j);
		}
	}

	for (uint i = 1; i < system_size; i += 2) {
		uint j = (i + 1) % system_size;
		if (!state->graph.contains_edge(i, j)) {
			state->toggle_edge_gate(i, j);
		} else {
			state->graph.local_complement(i);
			state->graph.local_complement(j);
		}
	}

	for (uint i = 0; i < system_size; i++) {
		// "measurement" step; with some probability, remove all edges incident on i
		if (randf() < mzr_prob) {
			for (auto const &j : state->graph.neighbors(i)) 
				state->toggle_edge_gate(i, j);
		}
	}
}

void SelfOrganizedCliffordSimulator::timesteps(uint num_steps) {
	switch (evolution_type) {
		case (EvolutionType::RandomClifford): rc_timesteps(num_steps); break;
		case (EvolutionType::QuantumAutomaton): qa_timesteps(num_steps); break;
		case (EvolutionType::GraphOperations): graph_timesteps(num_steps); break;
	}
}

void SelfOrganizedCliffordSimulator::add_distance_distribution(data_t &samples) const {
	std::vector<uint> hist(system_size/2, 0);
	for (uint i = 0; i < system_size; i++) {
		for (auto const &j : state->graph.neighbors(i))
			hist[dist(i,j)]++;
	}

	for (uint i = 0; i < system_size/2; i++)
		samples.emplace("dist_" + std::to_string(i), hist[i]);
}

data_t SelfOrganizedCliffordSimulator::take_samples() {
	data_t samples = EntropySimulator::take_samples();

	if (feedback_type == FeedbackType::ClusterThreshold) {
		samples.emplace("mzr_prob_f", mzr_prob);
		samples.emplace("max_cluster_size", max_component_size());
	} else if (feedback_type == FeedbackType::DistanceThreshold) {
		samples.emplace("mzr_prob_f", mzr_prob);
		samples.emplace("avg_distance", avg_dist());
	}

	add_distance_distribution(samples);

	return samples;
}