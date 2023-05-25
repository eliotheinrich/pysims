#ifndef UNITARY_STATE
#define UNITARY_STATE

#include "QuantumStatevector.hpp"
#include <Eigen/Dense>
#include <random>
#include <cmath>

// Apply gates to arbitrary qubits 👍
// Generate Haar random unitaries 👍
// Test Haar random unitaries 👍
// Perform partial trace and compute entropy 👍
// Generate Grover operations 
// TESTS
// Write simulator 
// Configure CMake 👍
// Simulate

class UnitaryState {
	private:
		uint num_qubits;
        std::minstd_rand rng;


	public:
		Eigen::MatrixXcd unitary;

		UnitaryState(uint num_qubits) : num_qubits(num_qubits) {
			unitary = Eigen::MatrixXcd::Zero(1u << num_qubits, 1u << num_qubits);
			unitary.setIdentity();
		}

		void apply_gate(const Eigen::MatrixXcd &gate, const std::vector<uint> &qubits) {
			Eigen::MatrixXcd full_gate = full_circuit_unitary(gate, qubits, num_qubits);
			apply_gate(full_gate);
		}

		void apply_gate(const Eigen::MatrixXcd &gate) {
			unitary = gate * unitary;
		}

		void normalize() {
			unitary = normalize_unitary(unitary);
		}

		QuantumStatevector get_statevector() const {
			QuantumStatevector statevector(num_qubits);
			statevector.apply_gate(unitary);
			return statevector;
		}

		float entropy(const std::vector<uint> &sites) const {
			// TODO avoid computing full density matrix
			return get_statevector().entropy(sites);
		}

		std::string to_string() const {
			// TODO display whole unitary state
			return get_statevector().to_string();
		}

		std::pair<double, double> probabilities(uint qubit) const {
			return get_statevector().probabilities(qubit);
		}
};

#endif