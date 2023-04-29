#include "SandpileCliffordSimulator.h"
#include <cmath>
#include <numeric>

#define DEFAULT_BOUNDARY_CONDITIONS "pbc"
#define DEFAULT_FEEDBACK_MODE 22
#define DEFAULT_RANDOM_SITES true

#define DEFAULT_SAMPLE_TRANSITION_MATRIX false


static inline BoundaryCondition parse_boundary_condition(std::string s) {
	if (s == "pbc") return BoundaryCondition::Periodic;
	else if (s == "obc1") return BoundaryCondition::Open1;
	else if (s == "obc2") return BoundaryCondition::Open2;
	else {
		std::cout << "Invalid boundary condition: " << s << std::endl;
		assert(false);
	}
}


SandpileCliffordSimulator::SandpileCliffordSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.get<float>("mzr_prob");
	unitary_prob = params.get<float>("unitary_prob");

	boundary_condition = parse_boundary_condition(params.get<std::string>("boundary_conditions", DEFAULT_BOUNDARY_CONDITIONS));
	feedback_mode = params.get<int>("feedback_mode", DEFAULT_FEEDBACK_MODE);

	random_sites = params.get<int>("random_sites", DEFAULT_RANDOM_SITES);

	system_size = params.get<int>("system_size");

	sample_transition_matrix = params.get<int>("sample_transition_matrix", DEFAULT_SAMPLE_TRANSITION_MATRIX);
	transition_matrix_unitary = std::vector<std::vector<uint>>(6, std::vector<uint>(6, 0));
	transition_matrix_mzr = std::vector<std::vector<uint>>(6, std::vector<uint>(6, 0));
	start_sampling = false;

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
		performed_mzr = true;

		state->mzr(i);
	}
}

void SandpileCliffordSimulator::unitary(uint i) {
	if (state->randf() < unitary_prob) {
		performed_unitary = true;

		uint q = i - direction;
		std::vector<uint> qubits{q, q + 1};
		state->random_clifford(qubits);
	}
}

void SandpileCliffordSimulator::timesteps(uint num_steps) {
	LOG("Calling SandpileCliffordSimulator::timesteps(" << num_steps << ")\n");
	for (uint k = 0; k < num_steps; k++)
		timestep();
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

uint SandpileCliffordSimulator::sp_cum_entropy_left(uint i) const {
	std::vector<uint> sites(i + 1);
	std::iota(sites.begin(), sites.end(), 0);
LOG("Left interval at " << i << ": " << print_vector(sites) << std::endl);
	return std::round(entropy(sites));
}

uint SandpileCliffordSimulator::sp_cum_entropy_right(uint i) const {
	std::vector<uint> sites(system_size - i);
	std::iota(sites.begin(), sites.end(), i);
LOG("Right interval at " << i << ": " << print_vector(sites) << std::endl);
	return std::round(entropy(sites));
}

uint SandpileCliffordSimulator::sp_cum_entropy(uint i) const {
	return direction ? sp_cum_entropy_right(i) : sp_cum_entropy_left(i);
}

uint SandpileCliffordSimulator::get_shape(uint s0, uint s1, uint s2) const {
	int ds1 = s0 - s1;
	int ds2 = s2 - s1;

	if      ((ds1 == 0)  && (ds2 == 0))   return 1;
	else if ((ds1 == -1) && (ds2 == -1))  return 2;
	else if ((ds1 == 1)  && (ds2 == 1))   return 3;
	else if ((ds1 == 0)  && (ds2 == 1))   return 4;
	else if ((ds1 == 1)  && (ds2 == 0))   return 4;
	else if ((ds1 == 0)  && (ds2 == -1))  return 5;
	else if ((ds1 == -1) && (ds2 == 0))   return 5;
	else if ((ds1 == -1) && (ds2 == 1))   return 6;
	else if ((ds1 == 1)  && (ds2 == -1))  return 6;
	else { std::cout << "Something has gone wrong.\n"; assert(false); }

	return -1;
}

void SandpileCliffordSimulator::feedback(uint q) {
	uint q0 = mod(q - 1, system_size);
	uint q2 = mod(q + 1, system_size);

	int s0 = sp_cum_entropy(q0);
	int s1 = sp_cum_entropy(q);
	int s2 = sp_cum_entropy(q2);

	uint shape = get_shape(s0, s1, s2);

	performed_mzr = false;
	performed_unitary = false;
	if (std::count(feedback_strategy.begin(), feedback_strategy.end(), shape)) 
		unitary(q);
	else 
		mzr(q);

	if (sample_transition_matrix && start_sampling) {
		if (performed_unitary || performed_mzr) {
			uint shape_after = get_shape(
				sp_cum_entropy(q0),
				sp_cum_entropy(q), 
				sp_cum_entropy(q2)
			);

			if (performed_unitary)
				transition_matrix_unitary[shape-1][shape_after-1]++;
			else if (performed_mzr)
				transition_matrix_mzr[shape-1][shape_after-1]++;
		}
	}
}

void SandpileCliffordSimulator::timestep() {
	left_boundary();

	direction = 0;
	for (uint i = 1; i < system_size-1; i++) {
		uint q = random_sites ? state->rand() % (system_size - 2) + 1 : i;
		feedback(q);
	}

	
	direction = 1;
	for (uint i = system_size-2; i > 0; i--) {
		uint q = random_sites ? state->rand() % (system_size - 2) + 1 : i;
		feedback(q);
	}

	right_boundary();

	direction = 0;
}

void SandpileCliffordSimulator::add_transition_matrix_samples(data_t &samples) {
	for (uint i = 0; i < 6; i++) {
		for (uint j = 0; j < 6; j++) {
			samples.emplace("transition_mzr_" + std::to_string(i) + "_" + std::to_string(j), transition_matrix_mzr[i][j]);
			samples.emplace("transition_unitary_" + std::to_string(i) + "_" + std::to_string(j), transition_matrix_unitary[i][j]);
		}
	}

	transition_matrix_unitary = std::vector<std::vector<uint>>(6, std::vector<uint>(6, 0));
	transition_matrix_mzr = std::vector<std::vector<uint>>(6, std::vector<uint>(6, 0));
}

data_t SandpileCliffordSimulator::take_samples() {
	data_t samples = EntropySimulator::take_samples();
	for (uint i = 0; i < system_size; i++)
		samples.emplace("entropy_" + std::to_string(i), sp_cum_entropy(i));
	
	if (sample_transition_matrix)
		add_transition_matrix_samples(samples);

	return samples;
}