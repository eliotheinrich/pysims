#include "MinCutSimulator.h"
#include "Graph.h"
#include <iostream>
//#include <boost/test/unit_test.hpp>



int main() {
    Graph g;
    g.add_vertex(0); g.add_vertex(0); g.add_vertex(0); 
    g.add_vertex(0); g.add_vertex(0); g.add_vertex(0); 

    g.add_directed_edge(0, 1, 16);
    g.add_directed_edge(0, 2, 13);
    g.add_directed_edge(1, 2, 10);
    g.add_directed_edge(1, 3, 12);
    g.add_directed_edge(2, 1, 4);
    g.add_directed_edge(2, 4, 14);
    g.add_directed_edge(3, 2, 9);
    g.add_directed_edge(3, 5, 20);
    g.add_directed_edge(4, 3, 7);
    g.add_directed_edge(4, 5, 4);

    std::cout << "Max flow: " << g.max_flow(0, 5) << std::endl;

    Params params;
    params.add("system_size", 8); params.add("partition_size", 2); params.add("mzr_prob", (float) 0.5);
    MinCutSimulator sim(params);
    sim.init_state();
    sim.timesteps(3);

    std::vector<uint> qubits{0,1};
    float s = sim.entropy(qubits);
    std::cout << "s = " << s << std::endl;
    


}





