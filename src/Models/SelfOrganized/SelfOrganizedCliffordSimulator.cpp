#include "SelfOrganizedCliffordSimulator.h"

#define DEFAULT_THRESHOLD 0.5
#define DEFAULT_EVOLUTION_TYPE "random_clifford"
#define DEFAULT_DX 0.05
#define DEFAULT_GATE_WIDTH 2


static EvolutionType parse_evolution_type(const std::string& s) {
	if (s == "random_clifford") {
		return EvolutionType::RandomClifford;
	} else if (s == "quantum_automaton") {
		return EvolutionType::QuantumAutomaton;
	} else if (s == "graph_operations") {
		return EvolutionType::GraphOperations;
	} else {
		std::string error_message = "Invalid evolution type: " + s;
		throw std::invalid_argument(error_message);
	}
}

static FeedbackType parse_feedback_type(const std::string& s) {
	if (s == "no_feedback") {
		return FeedbackType::NoFeedback;
	} else if (s == "cluster_threshold") {
		return FeedbackType::ClusterThreshold;
	} else if (s == "distance_threshold") {
		return FeedbackType::DistanceThreshold;
	} else {
		std::string error_message = "Invalid feedback type: " + s;
		throw std::invalid_argument(error_message);
	}
}

SelfOrganizedCliffordSimulator::SelfOrganizedCliffordSimulator(Params &params) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");

	mzr_prob = get<double>(params, "mzr_prob");
	x = std::log(mzr_prob/(1. - mzr_prob));

	threshold = get<double>(params, "threshold", DEFAULT_THRESHOLD);

	evolution_type = parse_evolution_type(get<std::string>(params, "evolution_type", DEFAULT_EVOLUTION_TYPE));
	feedback_type = parse_feedback_type(get<std::string>(params, "feedback_type"));
	dx = get<double>(params, "dx", DEFAULT_DX);

	gate_width = get<int>(params, "gate_width", DEFAULT_GATE_WIDTH);

	avalanche_size = 0;

	initial_offset = false;
}

void SelfOrganizedCliffordSimulator::init_state(uint32_t) {
	state = std::make_shared<QuantumGraphState>(system_size);
	if (evolution_type == EvolutionType::QuantumAutomaton) { // quantum automaton circuit must be polarized
		for (uint32_t i = 0; i < system_size; i++) state->h_gate(i);
	}
}

void SelfOrganizedCliffordSimulator::mzr(uint32_t q) {
	state->mzr(q);
	if (evolution_type == EvolutionType::QuantumAutomaton) {
		state->h_gate(q);
	}
}

uint32_t SelfOrganizedCliffordSimulator::dist(int i, int j) const {
	uint32_t d = std::abs(i - j);
	if (d > system_size/2) {
		return (system_size - d);
	} else {
		return d;
	}
}

float SelfOrganizedCliffordSimulator::avg_dist() const {
	float d = 0.;
	uint32_t n = 0;
	for (uint32_t i = 0; i < system_size; i++) {
		for (auto const &j : state->graph.neighbors(i)) {
			d += dist(i, j);
			n++;
		}
	}

	if (n == 0) {
		return 0.;
	} else {
		return d/n;
	}
}

float SelfOrganizedCliffordSimulator::max_component_size() const {
	return state->graph.max_component_size();
}

void SelfOrganizedCliffordSimulator::random_measure() {
	for (uint32_t j = 0; j < system_size; j++) {
		if (state->randf() < mzr_prob) {
			mzr(j);
		}
	}
}

void SelfOrganizedCliffordSimulator::cluster_threshold() {
	random_measure();

	if (max_component_size()/system_size > threshold) {
		x += dx;
	} else {
		x -= dx;
	}

	mzr_prob = 1./(1. + std::exp(-x));
}

void SelfOrganizedCliffordSimulator::distance_threshold() {
	random_measure();

	if (avg_dist()/system_size > threshold) {
		x += dx;
	} else {
		x -= dx;
	}

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
	for (uint32_t i = 0; i < system_size/2; i++) {
		uint32_t qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint32_t qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (rand() % 2 == 0) {
			std::swap(qubit1, qubit2);
		}

		if (gate_type) {
			state->cz_gate(qubit1, qubit2);
		} else {
			state->cx_gate(qubit1, qubit2);
		}
	}
}

void SelfOrganizedCliffordSimulator::qa_timesteps(uint32_t num_steps) {
	if (system_size % gate_width != 0) {
		throw std::invalid_argument("Invalid gate width. Must divide system size.");
	}

	for (uint32_t i = 0; i < num_steps; i++) {
		qa_timestep(false, false); // no offset, cx
		qa_timestep(false, true);  // no offset, cz
		qa_timestep(true, false);  // offset,    cx
		qa_timestep(true, true);   // offset,    cz

		mzr_feedback();
	}
}

void SelfOrganizedCliffordSimulator::rc_timesteps(uint32_t num_steps) {
	uint32_t num_qubits = system_size;
	if (system_size % gate_width != 0) {
		throw std::invalid_argument("Invalid gate width. Must divide system size.");
	} if (gate_width % 2 != 0) {
		throw std::invalid_argument("Gate width must be even.");
	}

	uint32_t num_gates = num_qubits / gate_width;
	bool offset_layer = initial_offset;

	std::vector<uint32_t> qubits(gate_width);
	std::iota(qubits.begin(), qubits.end(), 0);
	for (uint32_t i = 0; i < num_steps; i++) {
		for (uint32_t j = 0; j < num_gates; j++) {
			uint32_t offset = offset_layer ? gate_width*j : gate_width*j + gate_width/2;

			std::vector<uint32_t> offset_qubits(qubits);
			std::transform(offset_qubits.begin(), offset_qubits.end(), 
						offset_qubits.begin(), [num_qubits, offset](uint32_t x) { return (x + offset) % num_qubits; } );
			state->random_clifford(offset_qubits);
		}

		mzr_feedback();

		offset_layer = !offset_layer;
	}

	initial_offset = offset_layer;
}

void SelfOrganizedCliffordSimulator::graph_timesteps(uint32_t num_steps) {
	for (uint32_t i = 0; i < system_size; i += 2) {
		uint32_t j = (i + 1) % system_size;
		if (!state->graph.contains_edge(i, j)) {
			state->toggle_edge_gate(i, j);
		} else {
			state->graph.local_complement(i);
			state->graph.local_complement(j);
		}
	}

	for (uint32_t i = 1; i < system_size; i += 2) {
		uint32_t j = (i + 1) % system_size;
		if (!state->graph.contains_edge(i, j)) {
			state->toggle_edge_gate(i, j);
		} else {
			state->graph.local_complement(i);
			state->graph.local_complement(j);
		}
	}

	for (uint32_t i = 0; i < system_size; i++) {
		// "measurement" step; with some probability, remove all edges incident on i
		if (randf() < mzr_prob) {
			for (auto const &j : state->graph.neighbors(i)) {
				state->toggle_edge_gate(i, j);
			}
		}
	}
}

void SelfOrganizedCliffordSimulator::timesteps(uint32_t num_steps) {
	switch (evolution_type) {
		case (EvolutionType::RandomClifford): rc_timesteps(num_steps); break;
		case (EvolutionType::QuantumAutomaton): qa_timesteps(num_steps); break;
		case (EvolutionType::GraphOperations): graph_timesteps(num_steps); break;
	}
}

void SelfOrganizedCliffordSimulator::add_distance_distribution(data_t &samples) const {
	std::vector<uint32_t> hist(system_size/2, 0);
	for (uint32_t i = 0; i < system_size; i++) {
		for (auto const &j : state->graph.neighbors(i)) {
			hist[dist(i,j)]++;
		}
	}

	for (uint32_t i = 0; i < system_size/2; i++) {
		samples.emplace("dist_" + std::to_string(i), hist[i]);
	}
}

data_t SelfOrganizedCliffordSimulator::take_samples() {
	data_t samples;
	sampler.add_samples(samples, state);

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