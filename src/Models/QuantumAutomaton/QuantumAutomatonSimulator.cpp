#include "QuantumAutomatonSimulator.h"
#include <assert.h>

#define DEFAULT_CLIFFORD_TYPE "chp"
#define DEFAULT_SAMPLE_SURFACE false

#define QA_BRICKWORK 0
#define QA_POWERLAW 1

using namespace dataframe;
using namespace dataframe::utils;

QuantumAutomatonSimulator::QuantumAutomatonSimulator(ExperimentParams &params, uint32_t) : Simulator(params), entropy_sampler(params), interface_sampler(params) {
	system_size = get<int>(params, "system_size");
	clifford_type = parse_clifford_type(get<std::string>(params, "clifford_type", DEFAULT_CLIFFORD_TYPE));
	mzr_prob = get<double>(params, "mzr_prob");
	system_size = get<int>(params, "system_size");
	sample_surface = get<int>(params, "sample_surface", DEFAULT_SAMPLE_SURFACE);

	timestep_type = get<int>(params, "timestep_type", QA_BRICKWORK);
	if (timestep_type == QA_POWERLAW) {
		alpha = get<double>(params, "alpha", 2.0);
	}

	switch (clifford_type) {
		case CliffordType::CHP : state = std::make_shared<QuantumCHPState>(system_size); break;
		case CliffordType::GraphSim : state = std::make_shared<QuantumGraphState>(system_size); break;
	}

	// Initially polarize in x-direction
	for (uint32_t i = 0; i < system_size; i++) {
		state->h(i);
	}
}

void QuantumAutomatonSimulator::timesteps_brickwork(uint32_t num_steps) {
	for (uint32_t i = 0; i < num_steps; i++) {
		qa_timestep(state);

		for (uint32_t j = 0; j < system_size; j++) {
			if (randf() < mzr_prob) {
				state->mzr(j);
				state->h(j);
			}
		}
	}
}

uint32_t QuantumAutomatonSimulator::randpl() {
	return qa_power_law(1.0, system_size/2.0, -alpha, randf()); 
}

void QuantumAutomatonSimulator::timesteps_powerlaw(uint32_t num_steps) {
	for (uint32_t i = 0; i < num_steps; i++) {
		uint32_t q1 = randi() % system_size;
		uint32_t dq = randpl();
		uint32_t q2 = (randi() % 2) ? mod(q1 + dq, system_size) : mod(q1 - dq, system_size);

		if (randi() % 2) {
			if (randi() % 2) {
				state->cx(q1, q2);
			} else {
				state->cx(q2, q1);
			}
		} else {
			state->cz(q1, q2);
		}
	}
}

void QuantumAutomatonSimulator::timesteps(uint32_t num_steps) {
	assert(system_size % 2 == 0);

	if (timestep_type == QA_BRICKWORK) {
		timesteps_brickwork(num_steps);
	} else if (timestep_type == QA_POWERLAW) {
		timesteps_powerlaw(num_steps);
	}
}

SampleMap QuantumAutomatonSimulator::take_samples() {
	SampleMap samples;
	
	entropy_sampler.add_samples(samples, state);
	
	std::vector<int> surface = state->get_entropy_surface<int>(2);
	interface_sampler.add_samples(samples, surface);

	return samples;
}
