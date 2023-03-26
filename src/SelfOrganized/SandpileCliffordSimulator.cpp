#include "SandpileCliffordSimulator.h"
#include <cmath>
#include <numeric>

static BoundaryCondition parse_boundary_condition(std::string s) {
	if (s == "pbc") return BoundaryCondition::Periodic;
	else if (s == "obc1") return BoundaryCondition::Open1;
	else if (s == "obc2") return BoundaryCondition::Open2;
	else {
		std::cout << "Invalid boundary condition: " << s << std::endl;
		assert(false);
	}
}


SandpileCliffordSimulator::SandpileCliffordSimulator(Params &params) : Simulator(params) {
	mzr_prob = params.get<float>("mzr_prob");
	unitary_prob = params.get<float>("unitary_prob");

	boundary_condition = parse_boundary_condition(params.get<std::string>("boundary_conditions", DEFAULT_BOUNDARY_CONDITIONS));
	feedback_mode = params.get<int>("feedback_mode", DEFAULT_FEEDBACK_MODE);

	random_sites = params.get<int>("random_sites", DEFAULT_RANDOM_SITES);

	system_size = params.get<int>("system_size");

	if (feedback_mode == 0)       feedback_strategy = std::vector<uint>{1};
	else if (feedback_mode == 1)  feedback_strategy = std::vector<uint>{1, 2};
	else if (feedback_mode == 2)  feedback_strategy = std::vector<uint>{1, 3};
	else if (feedback_mode == 3)  feedback_strategy = std::vector<uint>{1, 4};
	else if (feedback_mode == 4)  feedback_strategy = std::vector<uint>{1, 5};
	else if (feedback_mode == 5)  feedback_strategy = std::vector<uint>{1, 6};
	else if (feedback_mode == 6)  feedback_strategy = std::vector<uint>{1, 2, 3};
	else if (feedback_mode == 7)  feedback_strategy = std::vector<uint>{1, 2, 4};
	else if (feedback_mode == 8)  feedback_strategy = std::vector<uint>{1, 2, 5};
	else if (feedback_mode == 9)  feedback_strategy = std::vector<uint>{1, 2, 6};
	else if (feedback_mode == 10) feedback_strategy = std::vector<uint>{1, 3, 4};
	else if (feedback_mode == 11) feedback_strategy = std::vector<uint>{1, 3, 5};
	else if (feedback_mode == 12) feedback_strategy = std::vector<uint>{1, 3, 6};
	else if (feedback_mode == 13) feedback_strategy = std::vector<uint>{1, 4, 5};
	else if (feedback_mode == 14) feedback_strategy = std::vector<uint>{1, 4, 6};
	else if (feedback_mode == 15) feedback_strategy = std::vector<uint>{1, 5, 6};
	else if (feedback_mode == 16) feedback_strategy = std::vector<uint>{1, 2, 3, 4};
	else if (feedback_mode == 17) feedback_strategy = std::vector<uint>{1, 2, 3, 5};
	else if (feedback_mode == 18) feedback_strategy = std::vector<uint>{1, 2, 3, 6};
	else if (feedback_mode == 19) feedback_strategy = std::vector<uint>{1, 2, 4, 5};
	else if (feedback_mode == 20) feedback_strategy = std::vector<uint>{1, 2, 4, 6};
	else if (feedback_mode == 21) feedback_strategy = std::vector<uint>{1, 2, 5, 6};
	else if (feedback_mode == 22) feedback_strategy = std::vector<uint>{1, 3, 4, 5};
	else if (feedback_mode == 23) feedback_strategy = std::vector<uint>{1, 3, 4, 6};
	else if (feedback_mode == 24) feedback_strategy = std::vector<uint>{1, 3, 5, 6};
	else if (feedback_mode == 25) feedback_strategy = std::vector<uint>{1, 4, 5, 6};
	else if (feedback_mode == 26) feedback_strategy = std::vector<uint>{1, 2, 3, 4, 5};
	else if (feedback_mode == 27) feedback_strategy = std::vector<uint>{1, 2, 3, 4, 6};
	else if (feedback_mode == 28) feedback_strategy = std::vector<uint>{1, 2, 3, 5, 6};
	else if (feedback_mode == 29) feedback_strategy = std::vector<uint>{1, 2, 4, 5, 6};
	else if (feedback_mode == 30) feedback_strategy = std::vector<uint>{1, 3, 4, 5, 6};
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
	switch (boundary_condition) {
		case (BoundaryCondition::Periodic): {
			std::vector<uint> qubits{0, 1};
			state->random_clifford(qubits);
			break;
		}
		case (BoundaryCondition::Open1): {
			break;
		}
		case (BoundaryCondition::Open2): {
			std::vector<uint> qubits{0, 1};
			state->random_clifford(qubits);
			break;
		}
	}
}

void SandpileCliffordSimulator::right_boundary() {
	switch (boundary_condition) {
		case (BoundaryCondition::Periodic): {
			std::vector<uint> qubits{system_size-1, 0};
			state->random_clifford(qubits);
			break;
		}
		case (BoundaryCondition::Open1): {
			break;
		}
		case (BoundaryCondition::Open2): {
			std::vector<uint> qubits{system_size-2, system_size-1};
			state->random_clifford(qubits);
			break;
		}
	}
}


void SandpileCliffordSimulator::feedback(int ds1, int ds2, uint q) {
	uint shape;
	if      ((ds1 == 0) && (ds2 == 0))   shape = 1;
	else if ((ds1 == -1) && (ds2 == -1)) shape = 2;
	else if ((ds1 == 1) && (ds2 == 1))   shape = 3;
	else if ((ds1 == 0) && (ds2 == 1))   shape = 4;
	else if ((ds1 == 1) && (ds2 == 0))   shape = 4;
	else if ((ds1 == 0) && (ds2 == -1))  shape = 5;
	else if ((ds1 == -1) && (ds2 == 0))  shape = 5;
	else if ((ds1 == -1) && (ds2 == 1))  shape = 6;
	else if ((ds1 == 1) && (ds2 == -1))  shape = 6;
	else { std::cout << "Something has gone wrong.\n"; assert(false); }


	if (std::count(feedback_strategy.begin(), feedback_strategy.end(), shape)) unitary(q);
	else mzr(q);
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