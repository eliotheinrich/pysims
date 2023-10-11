#include "PhaselessSimulator.h"

#define DEFAULT_DIM 1
#define DEFAULT_Z_PROB 0.5

PhaselessSimulator::PhaselessSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = get<double>(params, "mzr_prob");
	z_prob = get<double>(params, "z_prob", DEFAULT_Z_PROB);
	num_x_eigenstates = get<int>(params, "num_x_eigenstates", system_size/2);

	dim = get<int>(params, "dim", DEFAULT_DIM);
	
	sample_measurement_outcomes = get<int>(params, "sample_measurement_outcomes");
}

void PhaselessSimulator::init_state(uint32_t) {
	state = std::make_unique<QuantumCHPState<Tableau>>(system_size);
	offset = false;
}

data_t PhaselessSimulator::take_samples() {
	data_t samples = EntropySimulator::take_samples();

	if (sample_measurement_outcomes) {
		samples.emplace("mzr_expectation", state->CliffordState::mzr_expectation());
		samples.emplace("mxr_expectation", state->CliffordState::mxr_expectation());
	}

	return samples;
}

void PhaselessSimulator::one_dimensional_timestep() {
	for (uint32_t i = 0; i < system_size/2; i ++) {
		uint32_t qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint32_t qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (rand() % 2 == 0) std::swap(qubit1, qubit2);

		if (rand() % 2 == 0) state->swap_gate(qubit1, qubit2);
		else state->cx_gate(qubit1, qubit2);
	}

	for (uint32_t q = 0; q < system_size; q++) {
		if (randf() < mzr_prob) {
			if (randf() < z_prob) {
				state->mzr(q);
			} else {
				state->mxr(q);
			}
		}
	}

	offset = !offset;
}

void PhaselessSimulator::two_dimensional_timestep() {
	for (uint32_t i = 0; i < system_size/2; i ++) {
		uint32_t qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint32_t qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (rand() % 2 == 0) std::swap(qubit1, qubit2);

		if (rand() % 2 == 0) state->swap_gate(qubit1, qubit2);
		else state->cx_gate(qubit1, qubit2);
	}

	for (uint32_t q = 0; q < system_size; q++) {
		if (randf() < mzr_prob) {
			if (randf() < z_prob) {
				state->mzr(q);
			} else {
				state->mxr(q);
			}
		}
	}

	offset = !offset;
}

void PhaselessSimulator::timesteps(uint32_t num_steps) {
	if (dim == 1) {
		for (uint32_t i = 0; i < num_steps; i++)
			one_dimensional_timestep();
	} else if (dim == 2) {
		for (uint32_t i = 0; i < num_steps; i++)
			two_dimensional_timestep();
	}
}