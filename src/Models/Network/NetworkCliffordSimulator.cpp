#include "NetworkCliffordSimulator.h"

#define DEFAULT_NUM_PARTITIONS 10

using namespace dataframe;
using namespace dataframe::utils;

NetworkCliffordSimulator::NetworkCliffordSimulator(ExperimentParams &params, uint32_t) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");

	p = get<double>(params, "p");
	mzr_prob = get<double>(params, "mzr_prob");

	alpha = get<double>(params, "alpha");

	num_partitions = get<int>(params, "num_partition", DEFAULT_NUM_PARTITIONS);

	state = std::make_shared<QuantumCHPState>(system_size);
	network = UndirectedGraph<int>::scale_free_graph(system_size, alpha);
}

void NetworkCliffordSimulator::timesteps(uint32_t num_steps) {
	for (uint32_t k = 0; k < num_steps; k++) {
		for (uint32_t i = 0; i < system_size; i++) {
			uint32_t q = rand() % system_size;
			for (auto const &j : network.edges_of(q)) {
				if (randf() < p) {
					std::vector<uint32_t> qubits{q, j};
					state->random_clifford(qubits);
				}
			}
		}
	}
}

void NetworkCliffordSimulator::add_degree_distribution(SampleMap& samples) const {
	std::vector<uint32_t> dist(system_size);
	for (uint32_t i = 0; i < system_size; i++) {
		dist[network.degree(i)]++;
	}

	double sum = 0.0;
	for (uint32_t i = 0; i < system_size; i++) {
		sum += dist[i];
	}

	for (uint32_t i = 0; i < system_size; i++) {
    std::string key = "dist_" + std::to_string(i);
    emplace(samples, key, dist[i]/sum);
	}
}

void NetworkCliffordSimulator::add_spatially_averaged_entropy(SampleMap& samples) {
	std::vector<uint32_t> all_qubits(system_size);
	std::iota(all_qubits.begin(), all_qubits.end(), 0);

	uint32_t num_partitions = 10;
  std::vector<double> s(num_partitions);

	thread_local std::mt19937 gen(rand());
	for (uint32_t i = 0; i < num_partitions; i++) {
		std::shuffle(all_qubits.begin(), all_qubits.end(), gen);

		std::vector<uint32_t> qubits(all_qubits.begin(), all_qubits.begin() + system_size/2);
    s[i] = state->entanglement(qubits, 2);
	}

  emplace(samples, "entropy", s);
}

SampleMap NetworkCliffordSimulator::take_samples() {
	SampleMap samples;
	sampler.add_samples(samples, state);

	add_spatially_averaged_entropy(samples);

	return samples;
}
