#include "MinCutSimulator.h"
#include "Graph.h"
#include <iostream>
#include <assert.h>
//#include <boost/test/unit_test.hpp>

bool test_max_flow() {
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

    Params params;
    params.add("system_size", 8); params.add("partition_size", 2); params.add("mzr_prob", (float) 0.5);
    MinCutSimulator sim(params);
    sim.init_state();
    sim.timesteps(3);

    std::vector<uint> qubits{0,1};
    float s = sim.entropy(qubits);
    
    return s == 23;
}

#define HIND 0
#define SIND 1
#define CXIND 2
#define MZRIND 3

#define NUM_INSTRUCTIONS 4

#include "CliffordState.hpp"
#include "QuantumGraphState.h"
#include "QuantumCHPState.h"

class Instruction {
    public:
        uint system_size;
        uint i;
        uint q1;
        uint q2;

    Instruction(uint system_size) : system_size(system_size) {
        i = std::rand() % NUM_INSTRUCTIONS;
        q1 = std::rand() % system_size;
        q2 = std::rand() % system_size;
        while (q2 == q1) {
            q2 = std::rand() % system_size;
        }
    }

    Instruction(uint system_size, uint i, uint q1, uint q2) : system_size(system_size), i(i), q1(q1), q2(q2) {}

    void apply(CliffordState &state) {
        if (i == HIND) {
            state.h_gate(q1);
        } else if (i == SIND) {
            state.s_gate(q1);
        } else if (i == CXIND) {
            state.cx_gate(q1, q2);
        } else if (i == MZRIND) {
            state.mzr(q1);
        }
    }

    std::string to_string() {
        std::string s = "";
        if (i == HIND) {
            s += "h " + std::to_string(q1);
        } else if (i == SIND) {
            s += "s " + std::to_string(q1);
        } else if (i == CXIND) {
            s += "cx " + std::to_string(q1) + " " + std::to_string(q2);
        } else if (i == MZRIND) {
            s += "mzr " + std::to_string(q1);
        }

        return s;
    }
};

bool test_graph_entropy() {
    uint system_size = 6;
    uint num_gates = 1000;
    QuantumGraphState state1(system_size);
    QuantumCHPState state2(system_size);

    std::vector<uint> qubits{0,1,2,3,4};

    std::vector<Instruction> instructions;
    for (uint i = 0; i < num_gates; i++) instructions.push_back(Instruction(system_size));
    for (auto I : instructions) {
        std::cout << I.to_string() << std::endl;
        I.apply(state1);
        I.apply(state2);
    float s1 = state1.entropy(qubits);
    float s2 = state2.entropy(qubits);
    std::cout << s1 << " " << s2 << std::endl;
    std::cout << state1.to_string() << std::endl;
        assert(state1.entropy(qubits) == state2.entropy(qubits));
    }

    float s1 = state1.entropy(qubits);
    float s2 = state2.entropy(qubits);
    //std::cout << s1 << " " << s2 << std::endl;
    return s1 == s2;
}

bool fixed_graph_entropy() {
    uint system_size = 8;
    QuantumGraphState state(system_size);
    state.graph->set_val(0, 14);
    state.graph->set_val(1, 12);
    state.graph->set_val(2, 19);
    state.graph->set_val(3, 18);
    state.graph->set_val(4, 9);
    state.graph->set_val(5, 19);
    state.graph->set_val(6, 3);
    state.graph->set_val(7, 14);

    state.graph->add_edge(0, 1);
    state.graph->add_edge(0, 4);
    state.graph->add_edge(0, 7);
    state.graph->add_edge(1, 3);
    state.graph->add_edge(1, 4);
    state.graph->add_edge(1, 6);
    state.graph->add_edge(2, 3);
    state.graph->add_edge(3, 4);
    state.graph->add_edge(3, 7);
    state.graph->add_edge(4, 5);
    state.graph->add_edge(4, 6);
    state.graph->add_edge(4, 7);

    //std::cout << state.to_string() << std::endl;
    std::vector<uint> qubits{0,1,2,3,4,5};
    std::cout << state.entropy(qubits) << std::endl;
    return true;
}

int main() {
    fixed_graph_entropy();
}





