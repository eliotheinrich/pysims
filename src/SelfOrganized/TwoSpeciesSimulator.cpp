#include "TwoSpeciesSimulator.h"

#define DEFAULT_RANDOM_SITES true
#define DEFAULT_INITIAL_STATE 0

TwoSpeciesSimulator::TwoSpeciesSimulator(Params &params) : EntropySimulator(params) {
	p = params.get<float>("p");
	random_sites = params.get<int>("random_sites", DEFAULT_RANDOM_SITES);
}

void TwoSpeciesSimulator::init_state() { 
	state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(system_size)); 
	if (initial_state == 0) // Empty
		occupation = std::vector<int>(system_size, 0);
}

void TwoSpeciesSimulator::timestep() {

}

void TwoSpeciesSimulator::timesteps(uint num_steps) {
	for (uint k = 0; k < num_steps; k++)
		timestep();
}

data_t TwoSpeciesSimulator::take_samples() {
	data_t samples = EntropySimulator::take_samples();
	for (uint i = 0; i < system_size; i++)
		samples.emplace("entropy_" + std::to_string(i), cum_entropy(i));

	return samples;
}