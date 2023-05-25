#include "Graph.h"
#include <random>
#include <climits>

Graph::Graph(const Graph &g) : Graph() {
	for (uint i = 0; i < g.num_vertices; i++) add_vertex(DEFAULT_VAL);

	for (uint i = 0; i < g.num_vertices; i++) {
		for (auto const &[j, w] : g.edges[i]) add_directed_edge(i, j, w);
	}
}

std::string Graph::to_string() const {
	std::string s = "";
	for (uint i = 0; i < num_vertices; i++) {
		s += "[" + std::to_string(vals[i]) + "] " + std::to_string(i) + " -> ";
		for (auto const&[v, w] : edges[i]) {
			s += "(" + std::to_string(v) + ": "  + std::to_string(w) + ") ";
		}
		if (i != num_vertices - 1) s += "\n";
	}
	return s;
}

Graph Graph::erdos_renyi_graph(uint num_vertices, float p) {
	Graph g(num_vertices);
	thread_local std::minstd_rand r;
	for (uint i = 0; i < num_vertices-1; i++) {
		for (uint j = i+1; j < num_vertices; j++) {
			if (float(r())/float(RAND_MAX) < p)
				g.toggle_edge(i, j);
		}
	}

	return g;
}

void Graph::remove_vertex(uint v) {
	num_vertices--;
	edges.erase(edges.begin() + v);
	vals.erase(vals.begin() + v);
	for (uint i = 0; i < num_vertices; i++) edges[i].erase(v);

	for (uint i = 0; i < num_vertices; i++) {
		std::map<uint, int> new_edges;
		for (auto const &[j, w] : edges[i]) {
			if (j > v) new_edges.emplace(j-1, w);
			else new_edges.emplace(j, w);
		}
		edges[i] = new_edges;
	}
}

bool Graph::contains_edge(uint v1, uint v2) const {
	return contains_directed_edge(v1, v2) && contains_directed_edge(v2, v1);
}

int Graph::edge_weight(uint v1, uint v2) const {
	return edges[v1].at(v2);
}

bool Graph::contains_directed_edge(uint v1, uint v2) const {
	return edges[v1].count(v2);
}

void Graph::set_edge_weight(uint v1, uint v2, int w) {
	edges[v1][v2] = w;
}

void Graph::add_edge(uint v1, uint v2) {
	add_weighted_edge(v1, v2, 1);
}

void Graph::add_directed_edge(uint v1, uint v2, int w) {
	if (!contains_edge(v1, v2)) edges[v1].emplace(v2, w);
}

void Graph::add_weighted_edge(uint v1, uint v2, int w) {
	add_directed_edge(v1, v2, w);
	if (v1 != v2) add_directed_edge(v2, v1, w);
}

void Graph::remove_directed_edge(uint v1, uint v2) {
	edges[v1].erase(v2);
}

void Graph::remove_edge(uint v1, uint v2) {
	remove_directed_edge(v1, v2);
	if (v1 != v2) remove_directed_edge(v2, v1);
}

void Graph::toggle_directed_edge(uint v1, uint v2) {
	if (contains_directed_edge(v1, v2)) remove_directed_edge(v1, v2);
	else add_directed_edge(v1, v2, 1);
}

void Graph::toggle_edge(uint v1, uint v2) {
	toggle_directed_edge(v1, v2);
	if (v1 != v2) toggle_directed_edge(v2, v1);
}

int Graph::adjacency_matrix(uint v1, uint v2) const {
	if (contains_directed_edge(v1, v2)) return edges[v1].at(v2);
	else return 0;
}

uint Graph::degree(uint v) const {
	return edges[v].size();
}

void Graph::local_complement(uint v) {
	for (auto const &[v1, _] : edges[v]) {
		for (auto const &[v2, _] : edges[v]) {
			if (v1 < v2) toggle_edge(v1, v2);
		}
	}
}


Graph Graph::partition(const std::vector<uint> &nodes) const {
	std::set<uint> nodess;
	std::copy(nodes.begin(), nodes.end(), std::inserter(nodess, nodess.end()));
	Graph new_graph;
	std::map<uint, uint> new_vertices;

	for (const uint a : nodess) {
		if (!degree(a)) continue;

		new_vertices.emplace(a, new_vertices.size());
		new_graph.add_vertex(1);
		for (auto const &[b, _] : edges[a]) {
			if (nodess.count(b)) continue;

			if (!new_vertices.count(b)) {
				new_vertices.emplace(b, new_vertices.size());
				new_graph.add_vertex(0);
			}

			new_graph.add_edge(new_vertices[a], new_vertices[b]);
		}
	}

	bool continue_deleting = true;

	while (continue_deleting) {
		continue_deleting = false;
		for (uint i = 0; i < new_graph.num_vertices; i++) {
			if (!new_graph.degree(i)) {
				new_graph.remove_vertex(i);
				continue_deleting = true;
				break;
			}
		}
	}

	return new_graph;
}

std::pair<bool, std::vector<uint>> Graph::path(uint s, uint t) const {
	std::vector<uint> stack;
	stack.push_back(s);

	std::set<uint> visited;
	std::map<uint, uint> parent;

	while (!stack.empty()) {
		uint v = *(stack.end()-1);
		stack.pop_back();
		if (visited.count(v)) continue;

		visited.insert(v);
		for (auto const &[w, _] : edges[v]) {
			if (edge_weight(v, w) > 0) {
				if (!visited.count(w)) parent.emplace(w, v);
				if (w == t) {
					// Done; re-use stack
					stack.clear();
					uint node = t;
					while (parent.count(node)) {
						stack.push_back(node);
						node = parent[node];
					}
					stack.push_back(s);
					std::reverse(stack.begin(), stack.end());

					return std::pair(true, stack);
				}
				stack.push_back(w);
			}
		}
	}
	return std::pair(false, std::vector<uint>());
}

int Graph::max_flow(std::vector<uint> &sources, std::vector<uint> &sinks) const {
	Graph g(*this);

	g.add_vertex(DEFAULT_VAL);
	uint s = g.num_vertices - 1;
	g.add_vertex(DEFAULT_VAL);
	uint t = g.num_vertices - 1;

	for (auto i : sources) g.add_directed_edge(s, i, INT_MAX);
	for (auto i : sinks) g.add_directed_edge(i, t, INT_MAX);

	int flow = g.max_flow(s, t);

	return flow;
}

int Graph::max_flow(uint s, uint t) const {
	Graph residual_graph(*this);

	for (uint i = 0; i < num_vertices; i++) {
		for (auto const &[w, _] : residual_graph.edges[i]) {
			residual_graph.add_weighted_edge(i, w, 0);
		}
	}

	std::pair<bool, std::vector<uint>> p = residual_graph.path(s, t);
	bool path_exists = p.first;
	std::vector<uint> path_nodes = p.second;

	while (path_exists) {
		int min_weight = INT_MAX;
		for (uint j = 0; j < path_nodes.size() - 1; j++) {
			int weight = residual_graph.edge_weight(path_nodes[j], path_nodes[j+1]);
			if (weight < min_weight) min_weight = weight;
		}

		for (uint j = 0; j < path_nodes.size() - 1; j++) {
			uint u = path_nodes[j];
			uint v = path_nodes[j+1];
			residual_graph.set_edge_weight(v, u, residual_graph.edge_weight(v, u) + min_weight);
			residual_graph.set_edge_weight(u, v, residual_graph.edge_weight(u, v) - min_weight);
		}

		p = residual_graph.path(s, t);
		path_exists = p.first;
		path_nodes = p.second;
	}

	int flow = 0;
	for (auto const &[v, w] : residual_graph.edges[s]) {
		flow += edge_weight(s, v) - w;
	}

	return flow;
}

std::vector<uint> Graph::compute_degree_counts() const {
	std::vector<uint> counts(num_vertices, 0);
	for (uint i = 0; i < num_vertices; i++) counts[degree(i)]++;
	return counts;
}

std::vector<uint> Graph::compute_neighbor_degree_counts() const {
	// TODO thread safety?
	std::minstd_rand rng(std::rand());
	std::vector<uint> counts(num_vertices, 0);
	for (uint i = 0; i < num_vertices; i++) {
		if (degree(i) > 0) {
			uint v = rng() % degree(i);
			counts[degree(v)]++;
		}
	}
	return counts;
}

std::set<uint> Graph::component(uint i) const {
	std::vector<uint> stack;
	stack.push_back(i);
	std::set<uint> visited;

	while (!stack.empty()) {
		uint v = *(stack.end()-1);
		stack.pop_back();
		if (!visited.count(v)) {
			visited.insert(v);
			for (auto const &[w, _] : edges[v]) stack.push_back(w);
		}
	}

	return visited;
}

float Graph::average_component_size() const {
	std::set<uint> to_check;
	for (uint i = 0; i < num_vertices; i++) to_check.insert(i);
	float avg = 0.;

	while (!to_check.empty()) {
		uint i = *to_check.begin(); // pop
		to_check.erase(to_check.begin());

		auto connected_component = component(i);
		uint component_size = connected_component.size();
		for (auto v : connected_component) {
			if (to_check.count(v)) to_check.erase(v);
		}

		avg += component_size*component_size;
	}

	return avg/num_vertices;
}

uint Graph::max_component_size() const {
	std::set<uint> to_check;
	for (uint i = 0; i < num_vertices; i++) to_check.insert(i);

	uint max_cluster_size = 0;

	while (!to_check.empty()) {
		uint i = *to_check.begin(); // pop
		to_check.erase(to_check.begin());

		auto connected_component = component(i);
		uint component_size = connected_component.size();
		if (component_size > max_cluster_size) max_cluster_size = component_size;
		for (auto v : connected_component) {
			if (to_check.count(v)) to_check.erase(v);
		}
	}

	return max_cluster_size;
}

float Graph::local_clustering_coefficient(uint i) const {
	uint ki = degree(i);
	if (ki == 0 || ki == 1) return 0.;

	uint c = 0;
	for (uint j = 0; j < num_vertices; j++) {
		for (uint k = 0; k < num_vertices; k++) {
			c += adjacency_matrix(i, j)*adjacency_matrix(j, k)*adjacency_matrix(k, i);
		}
	}

	return c/(ki*(ki - 1));
}

float Graph::global_clustering_coefficient() const {
	float c = 0.;
	for (uint i = 0; i < num_vertices; i++) c += local_clustering_coefficient(i);
	return c/num_vertices;
}

float Graph::percolation_probability() const {
	std::set<uint> to_check;
	for (uint i = 0; i < num_vertices; i++) to_check.insert(i);

	uint max_cluster_size = 0;

	while (!to_check.empty()) {
		uint i = *to_check.begin(); // pop
		to_check.erase(to_check.begin());

		auto connected_component = component(i);
		uint component_size = connected_component.size();
		if (component_size > max_cluster_size) max_cluster_size = component_size;
		for (auto v : connected_component) {
			if (to_check.count(v)) to_check.erase(v);
		}
	}

	return float(max_cluster_size)/num_vertices;
}