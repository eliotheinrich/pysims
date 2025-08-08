#include "PhaselessSimulator.h"

#define DEFAULT_DIM 1
#define DEFAULT_Z_PROB 0.5

#define DEFAULT_SAMPLE_MEASUREMENT_OUTCOMES true

using namespace dataframe;
using namespace dataframe::utils;

PhaselessSimulator::PhaselessSimulator(ExperimentParams &params, uint32_t) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");

	mzr_prob = get<double>(params, "mzr_prob");
	z_prob = get<double>(params, "z_prob", DEFAULT_Z_PROB);
	num_x_eigenstates = get<int>(params, "num_x_eigenstates", system_size/2);

	dim = get<int>(params, "dim", DEFAULT_DIM);
	
	sample_measurement_outcomes = get<int>(params, "sample_measurement_outcomes", DEFAULT_SAMPLE_MEASUREMENT_OUTCOMES);

	state = std::make_shared<QuantumCHPState>(system_size);
	offset = false;

	std::vector<uint32_t> q(system_size);
	std::iota(q.begin(), q.end(), 0);
	std::shuffle(q.begin(), q.end(), rng);
	for (uint32_t i = 0; i < num_x_eigenstates; i++) {
		state->h(q[i]);
	}
}

void PhaselessSimulator::apply_gate(uint32_t q1, uint32_t q2) {
	if (rand() % 2) {
		state->swap(q1, q2);
	} else {
		if (rand() % 2) {
			state->cx(q1, q2);
		} else {
			state->cx(q2, q1);
		}
	}
}

void PhaselessSimulator::measure(uint32_t q) {
	if (randf() < mzr_prob) {
		if (randf() < z_prob) {
			state->mzr(q);
		} else {
			state->mxr(q);
		}
	}
}

void PhaselessSimulator::one_dimensional_timestep() {
	for (uint32_t i = 0; i < system_size/2; i ++) {
		uint32_t qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint32_t qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		apply_gate(qubit1, qubit2);
	}

	for (uint32_t q = 0; q < system_size; q++) {
		measure(q);
	}

	offset = !offset;
}

void PhaselessSimulator::two_dimensional_timestep() {
	uint32_t width = uint32_t(std::sqrt(system_size));
	for (uint32_t i = 0; i < system_size; i ++) {
		uint32_t qubit1 = i;
		uint32_t qubit2 = (i + 1) % width ? (i + 1) : (i + 1 - width);
		uint32_t qubit3 = (i + width >= system_size) ? (i + width - system_size) : (i + width);

		apply_gate(qubit1, qubit2);
		apply_gate(qubit1, qubit3);
	}

	for (uint32_t q = 0; q < system_size; q++) {
		measure(q);
	}
}

void PhaselessSimulator::timesteps(uint32_t num_steps) {
	if (dim == 1) {
		for (uint32_t i = 0; i < num_steps; i++) {
			one_dimensional_timestep();
		}
	} else if (dim == 2) {
		for (uint32_t i = 0; i < num_steps; i++) {
			two_dimensional_timestep();
		}
	}
}

SampleMap PhaselessSimulator::take_samples() {
	SampleMap samples;
	sampler.add_samples(samples, state);

	if (sample_measurement_outcomes) {
    dataframe::utils::emplace(samples, "mzr_expectation", state->CliffordState::mzr_expectation());
		dataframe::utils::emplace(samples, "mxr_expectation", state->CliffordState::mxr_expectation());
	}

	return samples;
}
