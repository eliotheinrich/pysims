#include "MinCutConfig.h"
#include <assert.h>

std::vector<MinCutConfig*> MinCutConfig::load_json(nlohmann::json data) {
	uint num_system_sizes = data["system_sizes"].size();
	uint num_partition_sizes = data["partition_sizes"].size();
	uint num_params = data["fparams"].size();
	uint num_runs = data.value("num_runs", DEFAULT_NUM_RUNS);
	uint num_configs = num_system_sizes*num_partition_sizes*num_params*num_runs;
	assert(num_configs > 0);

	std::map<std::string, int> iparams;
	std::map<std::string, float> fparams;

	std::vector<MinCutConfig*> configs(0);
    iparams["equilibration_steps"] = data.value("equilibration_steps", DEFAULT_EQUILIBRATION_STEPS);
    iparams["sampling_timesteps"] = data.value("sampling_timesteps", DEFAULT_SAMPLING_TIMESTEPS);
    iparams["measurement_freq"] = data.value("measurement_freq", DEFAULT_MEASUREMENT_FREQ);
    iparams["spacing"] = data.value("spacing", DEFAULT_SPACING);
    iparams["temporal_avg"] = data.value("temporal_avg", DEFAULT_TEMPORAL_AVG);
    
    for (int system_size : data["system_sizes"]) {
        for (int partition_size : data["partition_sizes"]) {
            iparams["partition_size"] = partition_size;
            iparams["system_size"] = system_size;

            for (auto json_fparams : data["fparams"]) {
                fparams["mzr_prob"] = json_fparams["mzr_prob"];
                for (int i = 0; i < num_runs; i++) {
                    configs.push_back(new MinCutConfig(iparams, fparams));
                }
            }
        }
    }

    std::string data_filename = data["filename"];

    return configs;
}

void MinCutConfig::init_state() {
	state = new Graph(system_size/2);
	rng = new std::minstd_rand(std::rand());
}

std::map<std::string, Sample> MinCutConfig::take_samples() {
	std::map<std::string, Sample> sample;
	sample.emplace("entropy", spatially_averaged_entropy());
	return sample;
}

MinCutConfig::MinCutConfig(std::map<std::string, int> iparams, std::map<std::string, float> fparams)
	: TimeConfig(iparams, fparams), Entropy(iparams, fparams) {

	mzr_prob = fparams["mzr_prob"];
}

float MinCutConfig::entropy(std::vector<uint> &qubits) const {
	assert(qubits.size() % 2 == 0);
	uint num_vertices = state->num_vertices;

	uint tsteps = num_vertices/(system_size/2) - 1;
	uint subsystem_size = qubits.size()/2;
	uint d = tsteps*system_size/2;

	std::vector<uint> subsystem_a;
	for (auto q : qubits)
		if (q % 2 == 0) subsystem_a.push_back(q/2 + d);

	std::vector<uint> subsystem_b(system_size/2);
	std::iota(subsystem_b.begin(), subsystem_b.end(), d);
	std::remove_if(subsystem_b.begin(), subsystem_b.end(), [&subsystem_a](uint q){ return !(q < subsystem_a.front() || q > subsystem_a.back()); } );

	return state->max_flow(subsystem_a, subsystem_b);
}

void MinCutConfig::timesteps(uint num_steps) {
	for (uint t = 0; t < num_steps; t++) {
		uint num_vertices = state->num_vertices;
		uint num_new_vertices = system_size/2;
		for (uint i = 0; i < num_new_vertices; i++) state->add_vertex();

		for (uint i = 0; i < num_new_vertices; i++) {
			uint v1 = num_vertices + i;
			uint col = i;
			uint row = (v1 - i) / num_new_vertices;

			uint next_col = offset ? (col + 1) % num_new_vertices : (col - 1) % num_new_vertices; // TODO EUCLIDIAN MODULO
			uint v2 = (row - 1)*num_new_vertices + col;
			uint v3 = (row - 1)*num_new_vertices + next_col;

			if (randf() < 1. - mzr_prob) state->add_edge(v1, v2);
			if (randf() < 1. - mzr_prob) state->add_edge(v1, v3);
		}

	}
	
	if (num_steps % 2 == 1) offset = !offset;
}
