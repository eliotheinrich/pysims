#include "SelfOrganizedCliffordSimulator.h"
#include "QuantumCHPState.h"
#include <cmath>
#include <numeric>

SelfOrganizedCliffordSimulator::SelfOrganizedCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.getf("mzr_prob");
	unitary_prob = params.getf("unitary_prob");

	random_sites = params.geti("random_sites", DEFAULT_RANDOM_SITES);
}

void SelfOrganizedCliffordSimulator::mzr(uint i) {
	if (state->randf() < mzr_prob) {
		state->mzr(i);
	}
}

void SelfOrganizedCliffordSimulator::unitary(uint i) {
	if (state->randf() < unitary_prob) {
		std::vector<uint> qubits{i, i + 1};
		state->random_clifford(qubits);
	}
}

int SelfOrganizedCliffordSimulator::cum_entropy(uint i) const {
	std::vector<uint> qubits(i+1);
	std::iota(qubits.begin(), qubits.end(), 0);

	return std::round(state->entropy(qubits));
}

void SelfOrganizedCliffordSimulator::timesteps(uint num_steps) {
	for (uint k = 0; k < num_steps; k++) {
		timestep();
	}
}

void SelfOrganizedCliffordSimulator::timestep() {
	unitary(0);
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

		//std::cout << "On qubit " << q1 << std::endl;
		//std::cout << "Surface: [ ";
		//for (uint j = 0; j < system_size; j++) {
		//	int s = cum_entropy(j);
		//	std::cout << cum_entropy(j) << " ";
		//} std::cout << "]\n";

		//std::cout << "s = " << s0 << " " << s1 << " " << s2 << std::endl;
		//std::cout << "ds = " << ds1 << " " << ds2 << std::endl;

		if      ((ds1 == -1) && (ds2 == -1)) mzr(q1);
		//else if ((ds1 == -1) && (ds2 == 0))  mzr(q); // Forgotten case
		else if ((ds1 == -1) && (ds2 == 1))  mzr(q1);
		//else if ((ds1 == 0) && (ds2 == -1))  mzr(q); // Forgotten case
		else if ((ds1 == 0) && (ds2 == 0))   unitary(q1);
		else if ((ds1 == 0) && (ds2 == 1))   unitary(q1);
		else if ((ds1 == 1) && (ds2 == -1))  mzr(q1);
		else if ((ds1 == 1) && (ds2 == 0))   unitary(q1);
		else if ((ds1 == 1) && (ds2 == 1))   unitary(q1);

		//std::cout << "s = " << cum_entropy(q0) << " " << cum_entropy(q1) << " " << cum_entropy(q2) << "\n\n";
	}
	unitary(system_size-2);
}