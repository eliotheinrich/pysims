#include "PartneringSimulator.h"
#include <math.h>

#define POWER_LAW 0
#define DELTA 1
#define PROXIMITY 2

#define DEFAULT_AFFINITY_TYPE POWER_LAW

using namespace dataframe;
using namespace dataframe::utils;

PartneringSimulator::PartneringSimulator(ExperimentParams &params, uint32_t) : Simulator(params) {
	num_nodes = get<int>(params, "num_nodes");
	affinity_type = get<int>(params, "affinity_type", DEFAULT_AFFINITY_TYPE);

	relaxation_time = get<double>(params, "relaxation_time");

	sample_global_properties = get<int>(params, "sample_global_properties", true);
	sample_local_properties = get<int>(params, "sample_local_properties", false);
	sample_affinity = get<int>(params, "sample_affinity", false);
	sample_counts = get<int>(params, "sample_counts", false);

	counts = std::vector<std::vector<uint32_t>>(num_nodes, std::vector<uint32_t>(num_nodes, 0));


	partner_graph = UndirectedGraph<int>(2*num_nodes);
	affinity_graph = UndirectedGraph<int, int>(2*num_nodes);
	augmented_graph = UndirectedGraph<int, int>(2*num_nodes);

	for (uint32_t i = 0; i < num_nodes; i++) {
		for (uint32_t j = num_nodes; j < 2*num_nodes; j++) {
			double affinity = 0.0;
			if (affinity_type == POWER_LAW) {
				affinity = power_law(1.0, 2.0, -5.0, randf()) - 1.0;
			} else if (affinity_type == DELTA) {
				affinity = (j == i + num_nodes) ? 1.0 : 1.0/num_nodes;
			} else if (affinity_type == PROXIMITY) {
				uint32_t d = std::abs(int(i - (j - num_nodes)));
				if (d > num_nodes/2) {
					d = num_nodes - d;
				}
				affinity = double(d)/num_nodes;
			}

			affinity_graph.add_edge(i, j, int(affinity*INT_MAX));

			augmented_graph.add_edge(i, j, 0);
		}
	}
}

void PartneringSimulator::timesteps(uint32_t num_steps) {
	for (uint32_t k = 0; k < num_steps; k++) {
		// Keep track of drawn nodes
		std::vector<uint32_t> drawn;

		// Drawing nodes choose partner in a random order
		std::vector<uint> first(num_nodes);
		std::iota(first.begin(), first.end(), 0);
		std::shuffle(first.begin(), first.end(), rng);

		for (uint32_t i = 0; i < num_nodes; i++) {
			uint32_t idx1 = first[i];
			std::vector<double> weights;
			for (uint32_t j = num_nodes; j < 2*num_nodes; j++) {
				double weight;
				if (std::count(drawn.begin(), drawn.end(), j)) {
					weight = 0.;
				} else {
					weight = affinity(idx1, j)*(1.0 - std::exp(-double(last_contact(idx1, j))/double(num_nodes*relaxation_time)));
				}

				weights.push_back(weight);
			}

			std::discrete_distribution<uint32_t> dist(weights.begin(), weights.end());
			uint32_t idx2 = dist(rng) + num_nodes;
			drawn.push_back(idx2);

			partner_graph.add_edge(idx1, idx2);
			augmented_graph.set_edge_weight(idx1, idx2, 0);
			augmented_graph.set_edge_weight(idx2, idx1, 0);
			if (start_sampling) {
				counts[idx1][idx2 - num_nodes]++;
			}
		}



		// Reduce weights of augmented graph
		for (uint32_t i = 0; i < num_nodes; i++) {
			for (uint32_t j = num_nodes; j < 2*num_nodes; j++) {
				uint32_t new_weight = augmented_graph.edge_weight(i, j) + 1;
				augmented_graph.set_edge_weight(i, j, new_weight);
				augmented_graph.set_edge_weight(j, i, new_weight);
			}
		}
	}
}

void PartneringSimulator::add_affinity_samples(SampleMap& samples) const {
	for (uint32_t i = 0; i < num_nodes; i++) {
		for (uint32_t j = num_nodes; j < 2*num_nodes; j++) {
      dataframe::utils::emplace(samples, fmt::format("affinity_{}_{}", i, j - num_nodes), affinity(i, j));
		}
	}
}

void PartneringSimulator::add_global_properties_samples(SampleMap& samples) const {
  dataframe::utils::emplace(samples, "global_clustering_coefficient", partner_graph.global_clustering_coefficient());
	dataframe::utils::emplace(samples, "percolation_probability", partner_graph.percolation_probability());
	dataframe::utils::emplace(samples, "max_component_size", partner_graph.max_component_size());
	dataframe::utils::emplace(samples, "average_component_size", partner_graph.average_component_size());
}

void PartneringSimulator::add_local_properties_samples(SampleMap& samples) const {
	auto degree_counts = partner_graph.compute_degree_counts();
	for (uint32_t i = 0; i < 2*num_nodes; i++) {
    dataframe::utils::emplace(samples, fmt::format("deg_{}", i), degree_counts[i]);
	}
}

void PartneringSimulator::add_counts_samples(SampleMap& samples) const {
	for (uint32_t i = 0; i < num_nodes; i++) {
		for (uint32_t j = 0; j < num_nodes; j++) {
      dataframe::utils::emplace(samples, fmt::format("count_{}_{}", i, j), counts[i][j]);
		}
	}
}

SampleMap PartneringSimulator::take_samples() {
	SampleMap samples;

	if (sample_affinity) {
		add_affinity_samples(samples);
	}

	if (sample_global_properties) {
		add_global_properties_samples(samples);
	}

	if (sample_local_properties) {
		add_local_properties_samples(samples);
	}

	if (sample_counts) {
		add_counts_samples(samples);
	}

	return samples;
}
