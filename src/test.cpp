#include "MinCutSimulator.h"
#include "DataFrame.hpp"
#include "QuantumGraphState.h"
#include "Graph.h"
#include <iostream>
#include <assert.h>
#include <DataFrame.hpp>
#include <ctpl.h>

bool test_graph_entropy() {
    uint system_size = 14;
    QuantumGraphState state(system_size);
    for (uint i = 0; i < system_size; i++) {
//        state.graph.add_edge(i, system_size - i - 1);
//        state.graph.add_edge(i, (i + 1) % system_size);
        state.h_gate(i);
    }

    state.cz_gate(2, 9);
    state.cz_gate(3, 10);

    state.cz_gate(3, 9);
    state.cz_gate(2, 10);

//    state.cz_gate(2, 7);

    std::cout << state.to_string() << std::endl;

    std::vector<uint> qubits(0);
    std::cout << state.entropy(qubits) << " ";
    for (uint i = 0; i < system_size; i++) {
        qubits.push_back(i);
        std::cout << state.entropy(qubits) << " ";
    } std::cout << std::endl;

    return true;
}


int main() {
    test_graph_entropy();
}





