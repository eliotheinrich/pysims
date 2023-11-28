#include "EnvironmentSimulator.h"

#define DEFAULT_ENV_DIM 1

EnvironmentSimulator::EnvironmentSimulator(Params &params) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");

	int_prob = get<double>(params, "int_prob");

	env_dim = get<int>(params, "env_dim", DEFAULT_ENV_DIM);
	if (env_dim == 1)
		env_size = system_size;
	else if (env_dim == 2)
		env_size = system_size*system_size;
	else
		throw std::invalid_argument("Environment interactions must be 1d or 2d.");

	params.emplace("env_size", (int) env_size);
}

void EnvironmentSimulator::init_state(uint32_t) {
	state = std::make_shared<QuantumCHPState>(system_size + env_size);

	offset = false;
}

void EnvironmentSimulator::one_dimensional_interactions() {
	for (uint32_t i = 0; i < system_size; i++) {
		for (uint32_t j = 0; j < env_size; j++) {
			if (randf() < int_prob) {
				std::vector<uint32_t> qubits{i, j};
				state->random_clifford(qubits);
			}
		}
	}
}

void EnvironmentSimulator::two_dimensional_interactions() {
	for (uint32_t i = 0; i < system_size; i++) {
		for (uint32_t j = 0; j < env_size; j++) {
			if (randf() < int_prob) {
				std::vector<uint32_t> qubits{i, j};
				state->random_clifford(qubits);
			}
		}
	}
}

void EnvironmentSimulator::timesteps(uint32_t num_steps) {
	for (uint32_t i = 0; i < num_steps; i++) {
		for (uint32_t j = 0; j < system_size/2; j++) {
			std::vector<uint32_t> qubits;
			if (offset)
				qubits = std::vector<uint32_t>{2*j, (2*j+1)%system_size};
			else
				qubits = std::vector<uint32_t>{(2*j+1)%system_size, (2*j+2)%system_size};
		
			state->random_clifford(qubits);
		}

		offset = ! offset;

		if (env_dim == 1)
			one_dimensional_interactions();
		else if (env_dim == 2)
			two_dimensional_interactions();
	}
}

data_t EnvironmentSimulator::take_samples() {
	data_t samples;
	sampler.add_samples(samples, state);
	return samples;
}
