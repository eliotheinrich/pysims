#ifndef CLIFFORD_GRAPH_H
#define CLIFFORD_GRAPH_H

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <assert.h>
#include <algorithm>

#define DEFAULT_VAL 0

class Graph {
	private:
		std::vector<std::map<uint, int>> edges;
		std::vector<uint> vals;

	public:
		uint num_vertices;

		Graph() : num_vertices(0) {}
		Graph(uint num_vertices) : num_vertices(0) { for (uint i = 0; i < num_vertices; i++) add_vertex(DEFAULT_VAL); }
		Graph(const Graph &g);

		std::string to_string() const;

		void add_vertex() { add_vertex(DEFAULT_VAL); }

		void add_vertex(uint v) {
			num_vertices++;
			edges.push_back(std::map<uint, int>());
			vals.push_back(v);
		}

		void remove_vertex(uint v);

		void set_val(uint v, uint val) {
			assert(v < num_vertices);
			vals[v] = val;
		}

		uint get_val(uint v) { return vals[v]; }

		std::vector<uint> neighbors(uint a) {
			std::vector<uint> neighbors;
			for (auto const &[e, _] : edges[a]) neighbors.push_back(e);
			std::sort(neighbors.begin(), neighbors.end());
			return neighbors;
		}

		bool contains_edge(uint v1, uint v2) const;
		int edge_weight(uint v1, uint v2) const;
		void set_edge_weight(uint v1, uint v2, int w);
		void add_edge(uint v1, uint v2);

		void add_directed_edge(uint v1, uint v2, int w);
		bool contains_directed_edge(uint v1, uint v2) const;
		void add_weighted_edge(uint v1, uint v2, int w);
		void remove_directed_edge(uint v1, uint v2);

		void remove_edge(uint v1, uint v2);
		void toggle_directed_edge(uint v1, uint v2);

		void toggle_edge(uint v1, uint v2);
		int adjacency_matrix(uint v1, uint v2) const;

		uint degree(uint v) const;

		void local_complement(uint v);

		Graph partition(std::vector<uint> &set) const;

		std::pair<bool, std::vector<uint>> path(uint s, uint t) const;

		int max_flow(std::vector<uint> &sources, std::vector<uint> &sinks) const;
		int max_flow(uint s, uint t) const;

		std::set<uint> component(uint i) const;

		// Graph properties
		std::vector<uint> compute_degree_counts() const;
		std::vector<uint> compute_neighbor_degree_counts() const;
		float average_component_size() const;
		uint max_component_size() const;
		float local_clustering_coefficient(uint v) const;
		float global_clustering_coefficient() const;
		float percolation_probability() const;
};

#endif