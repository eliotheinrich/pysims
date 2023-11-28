#include <iostream>
#include "QuantumState.h"
#include "GroverProjectionSimulator.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <unsupported/Eigen/MatrixFunctions>
#include <vector>
#include <map>
#include <Frame.h>
#include <InterfaceSampler.hpp>

std::string print_pair(std::pair<double, double> p) {
    return std::to_string(p.first) + ", " + std::to_string(p.second);
}

bool test_Statevector() {

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
          0, 0, 0, 1,
          0, 0, 1, 0,
          0, 1, 0, 0;

    //state.evolve(H, std::vector<uint32_t>{0});
    std::mt19937 rng;
    rng.seed(10);
    auto gate = haar_unitary(2, rng);
    std::cout << std::fixed << std::setprecision(12) << gate << std::endl << std::endl;
    Statevector state(3);
    state.evolve(gate, std::vector<uint32_t>{0, 2});
    std::cout << state.to_string() << "\n\n";
    //std::cout << state.entropy(std::vector<uint32_t>{0}, 2) << std::endl;
    //std::cout << state.entropy(std::vector<uint32_t>{1}, 2) << std::endl;
    //std::cout << state.entropy(std::vector<uint32_t>{2}, 2) << std::endl;
    //std::cout << print_pair(state.probabilities(0)) << std::endl;
    //std::cout << print_pair(state.probabilities(1)) << std::endl;
    //std::cout << print_pair(state.probabilities(2)) << std::endl;

    //state.evolve(CX, std::vector<uint32_t>{0, 1});
    //std::cout << state.to_string() << "\n\n";
    //std::cout << state.entropy(std::vector<uint32_t>{0}, 2) << std::endl;
    //std::cout << state.entropy(std::vector<uint32_t>{1}, 2) << std::endl;
    //std::cout << state.entropy(std::vector<uint32_t>{2}, 2) << std::endl;

    //std::cout << print_pair(state.probabilities(0)) << std::endl;
    //std::cout << print_pair(state.probabilities(1)) << std::endl;
    //std::cout << print_pair(state.probabilities(2)) << std::endl;

    return true;
}

bool test_GroverSimulation() {
    std::cout << "Beginning grover simulation\n";
    Params p;
    p["system_size"] = 3;
    p["mzr_prob"] = (double) 0.0;
    p["nmax"] = 50;

    GroverProjectionSimulator gs(p);
    gs.init_state(1);
    
    //for (uint32_t i = 0; i < 10; i++) {
    //    auto gate = haar_unitary(3);
    //    gs.state->evolve(haar_unitary(3));

    //    uint32_t q = gs.rand() % 3;
    //    gs.grover_projection(q, 1);

    //    auto probs = gs.state->probabilities(q);
    //    std::cout << gs.state->get_statevector().data << std::endl;
    //    std::cout << "on qubit " << q << ": " << print_pair(probs) << " sum to " << probs.first + probs.second << std::endl;
    //    std::cout << "entropy of projected qubit: " << gs.entropy(std::vector<uint32_t>{q}, 2) << "\n\n";
    //}

    std::cout << gs.create_oracle(3, std::vector<uint32_t>{1}) << std::endl;
    std::cout << gs.create_oracle(3, std::vector<uint32_t>{0,1}) << std::endl;
    std::cout << gs.create_oracle(3, std::vector<uint32_t>{0,1,2}) << std::endl;

    return true;
}

bool test_UnitaryEquivalence() {
    uint32_t num_qubits = 3;
    auto gate = haar_unitary(num_qubits);

    Statevector state_vector(num_qubits);
    UnitaryState state_unitary(num_qubits);

    state_vector.evolve(gate);
    state_unitary.evolve(gate);

    std::cout << state_vector.to_string() << std::endl;
    std::cout << state_unitary.to_string() << std::endl;

    return true;
}

bool test_normalize() {
    uint32_t num_qubits = 3;

    Statevector state(num_qubits);
    state.data = state.data/2;
    state.evolve(haar_unitary(num_qubits));

    std::cout << state.to_string() << std::endl;
    std::cout << state.norm() << std::endl;

    state.normalize();

    std::cout << state.to_string() << std::endl;
    std::cout << state.norm() << std::endl;


    std::cout << "\nBeginning UnitaryState tests\n\n";
    auto gate = haar_unitary(num_qubits);
    gate(0,0) += 1e-8;

    UnitaryState ustate1(num_qubits);
    ustate1.evolve(gate);

    for (auto e : ustate1.unitary.eigenvalues())
        std::cout << std::abs(e) << " ";
    std::cout << "\n\n";

    UnitaryState ustate2(num_qubits);
    ustate2.evolve(gate.pow(10000000));
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

bool test_density_matrix() {
    uint32_t num_qubits = 2;
    DensityMatrix d(num_qubits);
    std::cout << d.data << std::endl << std::endl;

    QuantumCircuit qc = generate_haar_circuit(num_qubits, 1);
    d.QuantumState::evolve(qc);

    std::cout << d.data << std::endl << std::endl;


    std::cout << d.data << std::endl << std::endl;

    return true;
}

#include <VQSE.hpp>
bool test_vqse() {
    uint32_t num_qubits = 4;
    std::cout << "Starting\n";
    QuantumCircuit target_circuit = generate_haar_circuit(num_qubits, 2);
    target_circuit.add_measurement(std::vector<uint32_t>{1});

    std::vector<std::string> rotation_gates{"rx", "ry", "rz"};
    QuantumCircuit ansatz = hardware_efficient_ansatz(num_qubits, 2, rotation_gates);
    std::vector<double> initial_params(ansatz.num_params());
    uint32_t m = 4;
    uint32_t max_iter = 1;

    VQSE vqse(ansatz, m, max_iter);
    std::cout << "vqse built\n";

    // vqse has taken ownership of ansatz; re-generate new copy.
    //ansatz = hardware_efficient_ansatz(num_qubits, 2);

    //vqse.optimize(target_circuit, initial_params);

    //for (auto e : vqse.true_eigenvalues(target_circuit)) std::cout << e << " ";
    //std::cout << "\n";

    //for (auto e : vqse.eigenvalue_estimates) std::cout << e << " ";
    //std::cout << "\n";
    for (uint32_t i = 0; i < num_qubits; i++) {
        std::cout << "r[" << i << "] = " << vqse.r[i] << std::endl;
        std::cout << "q[" << i << "] = " << vqse.q[i] << std::endl;
    }


    for (uint32_t i = 0; i < (1u << num_qubits); i++) {
        std::cout << i << ":" << std::endl;
        std::cout << "global: " << vqse.global_energy_by_bitstring(i) << std::endl;
        std::cout << "local: " << vqse.local_energy_by_bitstring(i) << std::endl;
    }


    return true;
}

bool test_vqse_energy() {
    uint32_t num_qubits = 8;
    for (uint32_t m = 1; m < 2; m++) {
        QuantumCircuit target_circuit = generate_haar_circuit(num_qubits, 2);
        target_circuit.add_measurement(std::vector<uint32_t>{1});
        DensityMatrix target(target_circuit);

        std::vector<std::string> rotation_gates{"rx", "ry", "rz"};
        QuantumCircuit ansatz = hardware_efficient_ansatz(num_qubits, 2, rotation_gates);
        std::vector<double> initial_params(ansatz.num_params());
        uint32_t max_iter = 500;

        VQSE vqse(std::move(ansatz), m, max_iter);

        auto local_energy_levels = vqse.get_local_energy_levels();
        std::vector<std::string> local_energy_levels_str;
        uint32_t s = m + 5;
        for (uint32_t i = 0; i < s; i++)
            local_energy_levels_str.push_back(std::to_string(local_energy_levels[i]));
        auto global_energy_levels = vqse.get_global_energy_levels();
        std::vector<std::string> global_energy_levels_str;
        for (uint32_t i = 0; i < s; i++)
            global_energy_levels_str.push_back(std::to_string(global_energy_levels[i]));

        std::cout << "m = " << m << std::endl;
        std::cout << "Local energy: " << join(local_energy_levels_str, " ") << std::endl;
        std::cout << "Global energy: " << join(global_energy_levels_str, " ") << std::endl;
    }
    return true;
}

#include <random>
#include "utils.cpp"
#include <sstream>

void print_params(std::vector<double> params) {
    std::cout << "[ ";
    for (auto p : params) std::cout << p << " ";
    std::cout << "]\n";
}

double grad_fd(QuantumCircuit qc, std::vector<double> params, Eigen::MatrixXcd obs, double epsilon=1e-3) {
    auto params_plus = params;
    params_plus[0] += epsilon;
    auto params_minus = params;
    params_minus[0] -= epsilon;

    print_params(params_plus);
    print_params(params_minus);
    
    double cost_plus = (DensityMatrix(qc.bind_params(params_plus)).data * obs).trace().real();
    double cost_minus = (DensityMatrix(qc.bind_params(params_minus)).data * obs).trace().real();

    return (cost_plus - cost_minus)/(2.0*epsilon);
}

double grad_ps(QuantumCircuit qc, std::vector<double> params, Eigen::MatrixXcd obs) {
    auto params_plus = params;
    params_plus[0] += PI/2;
    auto params_minus = params;
    params_minus[0] -= PI/2;
    
    print_params(params_plus);
    print_params(params_minus);

    double cost_plus = (DensityMatrix(qc.bind_params(params_plus)).data * obs).trace().real();
    double cost_minus = (DensityMatrix(qc.bind_params(params_minus)).data * obs).trace().real();

    return (cost_plus - cost_minus)/2.0;
}


bool test_parametrized_circuit() {
    std::vector<double> p{1.0};

    QuantumCircuit qc(1);
    qc.add_gate(parse_gate("Rx", std::vector<uint32_t>{0}));

    std::cout << qc.to_string() << std::endl;
    std::cout << "Gate op: \n";
    std::cout << qc.bind_params(p).to_matrix() << std::endl;

    Eigen::Matrix2cd Z; Z << 1.0, 0.0, 0.0, -1.0;

    std::cout << "grad_fd: " << grad_fd(qc, p, Z) << std::endl;
    std::cout << "grad_ps: " << grad_ps(qc, p, Z) << std::endl;

    return true;
}

#include <fstream>
#include <CircuitUtils.h>
#include <iomanip>


int main() {
    uint32_t system_size = 10;
    int seed = 314;
    QuantumCHPState state1(system_size, seed);
    QuantumGraphState state2(system_size, seed);

    std::random_device rng;

    uint32_t num_iters = 100;

    for (uint32_t i = 0; i < num_iters; i++) {
        uint32_t q1 = rng() % system_size;
        uint32_t q2;
        while (q2 == q1)
            q2 = rng() % system_size;

        std::vector<uint32_t> qbits{q1, q2};
        state1.random_clifford(qbits);
        state2.random_clifford(qbits);

        std::cout << "CHP state:   [ ";
        for (uint32_t i = 0; i < system_size; i++) {
            std::cout << state1.cum_entropy<int>(i) << " ";
        } std::cout << "]\n";

        std::cout << "Graph state: [ ";
        for (uint32_t i = 0; i < system_size; i++) {
            std::cout << state2.cum_entropy<int>(i) << " ";
        } std::cout << "]\n\n";
    }



}