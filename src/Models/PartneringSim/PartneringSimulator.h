#pragma once

#include <Frame.h>
#include <Graph.hpp>
#include <Simulator.hpp>

#include <climits>

static inline double power_law(double x0, double x1, double n, double r) {
	return std::pow(((std::pow(x1, n + 1.0) - std::pow(x0, n + 1.0))*r + std::pow(x0, n + 1.0)), 1.0/(n + 1.0));
}

class PartneringSimulator : public Simulator {
	private:
		uint32_t num_nodes;
		uint32_t affinity_type;

		double relaxation_time;

		bool sample_global_properties;
		bool sample_local_properties;
		bool sample_affinity;
		bool sample_counts;

		bool start_sampling = false;


		std::vector<std::vector<uint32_t>> counts;

		double affinity(uint32_t i, uint32_t j) const {
			return double(affinity_graph.edge_weight(i, j))/INT_MAX;
		}

		double last_contact(uint32_t i, uint32_t j) const {
			return augmented_graph.edge_weight(i, j);
		}

		void add_affinity_samples(dataframe::SampleMap& samples) const;
		void add_global_properties_samples(dataframe::SampleMap& samples) const;
		void add_local_properties_samples(dataframe::SampleMap& samples) const;
		void add_counts_samples(dataframe::SampleMap& samples) const;

	public:
		UndirectedGraph<int> partner_graph;
		UndirectedGraph<int, int> affinity_graph;
		UndirectedGraph<int, int> augmented_graph;

		PartneringSimulator(dataframe::ExperimentParams &params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;
		virtual void equilibration_timesteps(uint32_t num_steps) override {
			start_sampling = false;
			timesteps(num_steps);
			start_sampling = true;
		}

		virtual dataframe::SampleMap take_samples() override;
};
