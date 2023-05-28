#include <iostream>
#include "QuantumStatevector.hpp"
#include "GroverProjectionSimulator.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <unsupported/Eigen/MatrixFunctions>
#include <vector>
#include <map>
#include <DataFrame.hpp>

/*
std::string print_pair(std::pair<double, double> p) {
    return std::to_string(p.first) + ", " + std::to_string(p.second);
}

bool test_QuantumStatevector() {
    QuantumStatevector state(3);

    Eigen::MatrixXcd X(2, 2);
    X << 0, 1, 1, 0;

    Eigen::MatrixXcd Y(2, 2);
    Y << 0, -1, 1, 0;
    Y *= std::complex<double>(0, 1);

    Eigen::MatrixXcd Z(2, 2);
    Z << 1, 0, 0, -1;

    Eigen::MatrixXcd H(2, 2);
    H << 1, 1, 1, -1;
    H /= std::sqrt(2);

    Eigen::MatrixXcd CX(4, 4);
    CX << 1, 0, 0, 0,
          0, 1, 0, 0,
          0, 0, 0, 1,
          0, 0, 1, 0;

    state.apply_gate(H, std::vector<uint>{0});
    std::cout << state.to_string() << "\n\n";
    std::cout << state.entropy(std::vector<uint>{0}, 2) << std::endl;
    std::cout << state.entropy(std::vector<uint>{1}, 2) << std::endl;
    std::cout << state.entropy(std::vector<uint>{2}, 2) << std::endl;
    std::cout << print_pair(state.probabilities(0)) << std::endl;
    std::cout << print_pair(state.probabilities(1)) << std::endl;
    std::cout << print_pair(state.probabilities(2)) << std::endl;

    state.apply_gate(CX, std::vector<uint>{1, 0});
    std::cout << state.to_string() << "\n\n";
    std::cout << state.entropy(std::vector<uint>{0}, 2) << std::endl;
    std::cout << state.entropy(std::vector<uint>{1}, 2) << std::endl;
    std::cout << state.entropy(std::vector<uint>{2}, 2) << std::endl;

    std::cout << print_pair(state.probabilities(0)) << std::endl;
    std::cout << print_pair(state.probabilities(1)) << std::endl;
    std::cout << print_pair(state.probabilities(2)) << std::endl;

    return true;
}

bool test_GroverSimulation() {
    std::cout << "Beginning grover simulation\n";
    Params p;
    p.add("system_size", (int) 3);
    p.add("mzr_prob", (float) 0.0);
    p.add("nmax", (int) 50);

    GroverProjectionSimulator gs(p);
    gs.init_state();
    
    for (uint i = 0; i < 10; i++) {
        auto gate = haar_unitary(3);
        gs.state->apply_gate(haar_unitary(3));

        uint q = gs.rand() % 3;
        gs.grover_projection(q, 1);

        auto probs = gs.state->probabilities(q);
        std::cout << gs.state->get_statevector().data << std::endl;
        std::cout << "on qubit " << q << ": " << print_pair(probs) << " sum to " << probs.first + probs.second << std::endl;
        std::cout << "entropy of projected qubit: " << gs.entropy(std::vector<uint>{q}, 2) << "\n\n";
    }

    return true;
}

bool test_UnitaryEquivalence() {
    uint num_qubits = 3;
    auto gate = haar_unitary(num_qubits);

    QuantumStatevector state_vector(num_qubits);
    UnitaryState state_unitary(num_qubits);

    state_vector.apply_gate(gate);
    state_unitary.apply_gate(gate);

    std::cout << state_vector.to_string() << std::endl;
    std::cout << state_unitary.to_string() << std::endl;

    return true;
}

bool test_normalize() {
    uint num_qubits = 3;

    QuantumStatevector state(num_qubits);
    state.data = state.data/2;
    state.apply_gate(haar_unitary(num_qubits));

    std::cout << state.to_string() << std::endl;
    std::cout << state.norm() << std::endl;

    state.normalize();

    std::cout << state.to_string() << std::endl;
    std::cout << state.norm() << std::endl;


    std::cout << "\nBeginning UnitaryState tests\n\n";
    auto gate = haar_unitary(num_qubits);
    gate(0,0) += 1e-8;

    UnitaryState ustate1(num_qubits);
    ustate1.apply_gate(gate);

    for (auto e : ustate1.unitary.eigenvalues())
        std::cout << std::abs(e) << " ";
    std::cout << "\n\n";

    UnitaryState ustate2(num_qubits);
    ustate2.apply_gate(gate.pow(10000000));
    for (auto e : ustate2.unitary.eigenvalues())
        std::cout << std::abs(e) << " ";
    std::cout << "\n";
    std::cout << ustate2.get_statevector().to_string();
    std::cout << "\n\n";

    ustate2.normalize();
    for (auto e : ustate2.unitary.eigenvalues())
        std::cout << std::abs(e) << " ";
    std::cout << "\n";
    std::cout << ustate2.get_statevector().to_string();
    std::cout << "\n\n";

    return true;
}

#include <omp.h>
bool test_OMP() {
    #pragma omp parallel 
    {
        printf("Hello world from thread %d\n", omp_get_thread_num());
    }

    return true;
}
*/

#include <vector>
#include <iostream>



