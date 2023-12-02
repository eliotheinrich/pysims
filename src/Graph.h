#pragma once

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <assert.h>
#include <algorithm>

#define DEFAULT_VAL 0

class Graph {
	public:
		std::vector<std::map<uint32_t, int>> edges;
		std::vector<uint32_t> vals;

		uint32_t num_vertices;

		Graph() : num_vertices(0) {}
		Graph(uint32_t num_vertices) : num_vertices(0) { 
			for (uint32_t i = 0; i < num_vertices; i++) {
				add_vertex(DEFAULT_VAL);
			}
		}
		Graph(const Graph &g);

		static Graph erdos_renyi_graph(uint32_t num_vertices, float p);
		static Graph scale_free_graph(uint32_t num_vertices, float alpha);

		std::string to_string() const;

		void add_vertex() { 
			add_vertex(DEFAULT_VAL); 
		}

		void add_vertex(uint32_t v) {
			num_vertices++;
			edges.push_back(std::map<uint32_t, int>());
			vals.push_back(v);
		}

		void remove_vertex(uint32_t v);

		void set_val(uint32_t v, uint32_t val) {
			assert(v < num_vertices);
			vals[v] = val;
		}

		uint32_t get_val(uint32_t v) const { 
			return vals[v]; 
		}

		std::vector<uint32_t> neighbors(uint32_t a) const {
			std::vector<uint32_t> neighbors;
			for (auto const &[e, _] : edges[a]) {
				neighbors.push_back(e);
			}
			std::sort(neighbors.begin(), neighbors.end());
			return neighbors;
		}

		bool contains_edge(uint32_t v1, uint32_t v2) const;
		int edge_weight(uint32_t v1, uint32_t v2) const;
		void set_edge_weight(uint32_t v1, uint32_t v2, int w);
		void add_edge(uint32_t v1, uint32_t v2);

		void add_directed_edge(uint32_t v1, uint32_t v2, int w);
		bool contains_directed_edge(uint32_t v1, uint32_t v2) const;
		void add_weighted_edge(uint32_t v1, uint32_t v2, int w);
		void remove_directed_edge(uint32_t v1, uint32_t v2);

		void remove_edge(uint32_t v1, uint32_t v2);
		void toggle_directed_edge(uint32_t v1, uint32_t v2);

		void toggle_edge(uint32_t v1, uint32_t v2);
		int adjacency_matrix(uint32_t v1, uint32_t v2) const;

		uint32_t degree(uint32_t v) const;

		void local_complement(uint32_t v);

		Graph partition(const std::vector<uint32_t> &set) const;

		std::pair<bool, std::vector<uint32_t>> path(uint32_t s, uint32_t t) const;

		int max_flow(std::vector<uint32_t> &sources, std::vector<uint32_t> &sinks) const;
		int max_flow(uint32_t s, uint32_t t) const;

		std::set<uint32_t> component(uint32_t i) const;

		// Graph properties
		std::vector<uint32_t> compute_degree_counts() const;
		std::vector<uint32_t> compute_neighbor_degree_counts() const;
		double average_component_size() const;
		uint32_t max_component_size() const;
		double local_clustering_coefficient(uint32_t v) const;
		double global_clustering_coefficient() const;
		double percolation_probability() const;
};