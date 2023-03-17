#include "SandpileCliffordSimulator.h"
#include <cmath>
#include <numeric>

SandpileCliffordSimulator::SandpileCliffordSimulator(Params &params) : Simulator(params) {
	mzr_prob = params.getf("mzr_prob");
	unitary_prob = params.getf("unitary_prob");

	boundary_conditions = params.geti("boundary_conditions", DEFAULT_BOUNDARY_CONDITIONS);
	random_sites = params.geti("random_sites", DEFAULT_RANDOM_SITES);
	feedback_mode = params.geti("feedback_mode", DEFAULT_FEEDBACK_MODE);

	system_size = params.geti("system_size");
}

void SandpileCliffordSimulator::mzr(uint i) {
	if (state->randf() < mzr_prob) {
		state->mzr(i);
	}
}

void SandpileCliffordSimulator::unitary(uint i) {
	if (state->randf() < unitary_prob) {
		std::vector<uint> qubits{i, i + 1};
		state->random_clifford(qubits);
	}
}

int SandpileCliffordSimulator::cum_entropy(uint i) const {
	std::vector<uint> qubits(i+1);
	std::iota(qubits.begin(), qubits.end(), 0);

	return std::round(state->entropy(qubits));
}

void SandpileCliffordSimulator::timesteps(uint num_steps) {
	for (uint k = 0; k < num_steps; k++) {
		timestep();
	}
}

void SandpileCliffordSimulator::left_boundary() {
	if (boundary_conditions == 0) { // pbc
		std::vector<uint> qubits{0, 1};
		state->random_clifford(qubits);
	} else if (boundary_conditions == 1) { //obc1

	} else if (boundary_conditions == 2) { // obc2
		std::vector<uint> qubits{0, 1};
		state->random_clifford(qubits);
	} else {
		std::cout << "Invalid boundary conditions " << boundary_conditions << std::endl;
		assert(false);
	}
}

void SandpileCliffordSimulator::right_boundary() {
	if (boundary_conditions == 0) { // pbc
		std::vector<uint> qubits{system_size-1, 0};
		state->random_clifford(qubits);
	} else if (boundary_conditions == 1) { //obc1

	} else if (boundary_conditions == 2) { // obc2
		std::vector<uint> qubits{system_size-2, system_size-1};
		state->random_clifford(qubits);
	} else {
		std::cout << "Invalid boundary conditions " << boundary_conditions << std::endl;
		assert(false);
	}

}


void SandpileCliffordSimulator::feedback(int ds1, int ds2, uint q) {
	if (feedback_mode == 0) {
		if      ((ds1 == -1) && (ds2 == -1)) mzr(q);
		else if ((ds1 == -1) && (ds2 == 0))  unitary(q); 
		else if ((ds1 == -1) && (ds2 == 1))  mzr(q);
		else if ((ds1 == 0) && (ds2 == -1))  unitary(q); 
		else if ((ds1 == 0) && (ds2 == 0))   unitary(q);
		else if ((ds1 == 0) && (ds2 == 1))   unitary(q);
		else if ((ds1 == 1) && (ds2 == -1))  mzr(q);
		else if ((ds1 == 1) && (ds2 == 0))   unitary(q);
		else if ((ds1 == 1) && (ds2 == 1))   unitary(q);
		else {
			std::cout << "Detected |slope| > 1\n";
			assert(false);
		}
	} else if (feedback_mode == 1) {
		if      ((ds1 == -1) && (ds2 == -1)) unitary(q); // Different from feedback0
		else if ((ds1 == -1) && (ds2 == 0))  unitary(q); 
		else if ((ds1 == -1) && (ds2 == 1))  mzr(q);
		else if ((ds1 == 0) && (ds2 == -1))  unitary(q); 
		else if ((ds1 == 0) && (ds2 == 0))   unitary(q);
		else if ((ds1 == 0) && (ds2 == 1))   unitary(q);
		else if ((ds1 == 1) && (ds2 == -1))  mzr(q);
		else if ((ds1 == 1) && (ds2 == 0))   unitary(q);
		else if ((ds1 == 1) && (ds2 == 1))   unitary(q);
		else {
			std::cout << "Detected |slope| > 1\n";
			assert(false);
		}
	} else if (feedback_mode == 2) {
		if      ((ds1 == -1) && (ds2 == -1)) mzr(q);
		else if ((ds1 == -1) && (ds2 == 0))  unitary(q); 
		else if ((ds1 == -1) && (ds2 == 1))  mzr(q);
		else if ((ds1 == 0) && (ds2 == -1))  unitary(q); 
		else if ((ds1 == 0) && (ds2 == 0))   mzr(q);
		else if ((ds1 == 0) && (ds2 == 1))   unitary(q);
		else if ((ds1 == 1) && (ds2 == -1))  mzr(q);
		else if ((ds1 == 1) && (ds2 == 0))   unitary(q);
		else if ((ds1 == 1) && (ds2 == 1))   unitary(q);
		else {
			std::cout << "Detected |slope| > 1\n";
			assert(false);
		}
	} else {
		std::cout << "Invalid feedback mode " << feedback_mode << std::endl;
		assert(false);
	}
}

void SandpileCliffordSimulator::timestep() {
	left_boundary();

	for (uint i = 1; i < system_size-1; i++) {
		uint q1;
		if (random_sites) q1 = state->rand() % (system_size - 2) + 1;
		else q1 = i;
		uint q0 = mod(q1 - 1, system_size);
		uint q2 = mod(q1 + 1, system_size);

		int s0 = cum_entropy(q0);
		int s1 = cum_entropy(q1);
		int s2 = cum_entropy(q2);

		int ds1 = s0 - s1;
		int ds2 = s2 - s1;

		feedback(ds1, ds2, q1);
	}

	right_boundary();
}

std::map<std::string, Sample> SandpileCliffordSimulator::take_samples() {
	std::map<std::string, Sample> samples;
	for (uint i = 0; i < system_size; i++) {
		samples.emplace("entropy_" + std::to_string(i), cum_entropy(i));
	}

	return samples;
}