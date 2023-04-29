#include "BellSandpileSimulator.h"
#include <cmath>
#include <numeric>

#define DEFAULT_RANDOM_SITES true
#define DEFAULT_FEEDBACK_MODE 0
#define DEFAULT_GATE_TYPE 0

BellSandpileSimulator::BellSandpileSimulator(Params &params) : EntropySimulator(params) {
	pu = params.get<float>("pu");
	pm = params.get<float>("pm");

	random_sites = params.get<int>("random_sites", DEFAULT_RANDOM_SITES);

	gate_type = params.get<int>("gate_type", DEFAULT_GATE_TYPE);

	feedback_mode = params.get<int>("feedback_mode", DEFAULT_FEEDBACK_MODE);

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

void BellSandpileSimulator::unitary(uint i) {
	uint q1 = i;
	uint q2 = 2*system_size - i - 1;
	
	if (gate_type == 0)
		state->cz_gate(q1, q2);
	else if (gate_type == 1) {
		std::vector<uint> qubits{q1, q2};
		state->random_clifford(qubits);
	}
}

void BellSandpileSimulator::mzr(uint i) {
	state->mzr(i);
}

void BellSandpileSimulator::timesteps(uint num_steps) {
	LOG("Calling BellSandpileSimulator::timesteps(" << num_steps << ")\n");
	for (uint k = 0; k < num_steps; k++)
		timestep();
}

uint BellSandpileSimulator::get_shape(uint s0, uint s1, uint s2) const {
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

void BellSandpileSimulator::feedback(uint q) {
	uint q0 = mod(q - 1, system_size);
	uint q2 = mod(q + 1, system_size);

	int s0 = cum_entropy(q0);
	int s1 = cum_entropy(q);
	int s2 = cum_entropy(q2);

	uint shape = get_shape(s0, s1, s2);

	if (std::count(feedback_strategy.begin(), feedback_strategy.end(), shape) && randf() < pu) 
		unitary(q);
	else if (randf() < pm)
		mzr(q);
}

void BellSandpileSimulator::timestep() {
	for (uint i = 1; i < system_size-1; i++) {
		uint q = random_sites ? state->rand() % (system_size - 2) + 1 : i;
		feedback(q);
	}
}

data_t BellSandpileSimulator::take_samples() {
	data_t samples = EntropySimulator::take_samples();
	for (uint i = 0; i < system_size; i++)
		samples.emplace("entropy_" + std::to_string(i), cum_entropy(i));

	return samples;
}