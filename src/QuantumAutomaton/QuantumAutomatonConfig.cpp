#include "QuantumAutomatonConfig.h"
#include "Simulator.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<QuantumAutomatonConfig*> QuantumAutomatonConfig::load_json(json data) {
	uint num_system_sizes = data["system_sizes"].size();
	uint num_partition_sizes = data["partition_sizes"].size();
	uint num_params = data["fparams"].size();
	uint num_runs = data.value("num_runs", DEFAULT_NUM_RUNS);
	uint num_configs = num_system_sizes*num_partition_sizes*num_params*num_runs;
	assert(num_configs > 0);

    std::map<std::string, int> iparams;
    std::map<std::string, float> fparams;

    std::vector<QuantumAutomatonConfig*> configs(0);
    iparams["equilibration_steps"] = data.value("equilibration_steps", DEFAULT_EQUILIBRATION_STEPS);
    iparams["sampling_timesteps"] = data.value("sampling_timesteps", DEFAULT_SAMPLING_TIMESTEPS);
    iparams["measurement_freq"] = data.value("measurement_freq", DEFAULT_MEASUREMENT_FREQ);
    iparams["spacing"] = data.value("spacing", DEFAULT_SPACING);
    iparams["simulator_type"] = parse_clifford_type(data.value("simulator_type", DEFAULT_SIMULATOR));
    iparams["temporal_avg"] = data.value("temporal_avg", DEFAULT_TEMPORAL_AVG);
    
    for (int system_size : data["system_sizes"]) {
        for (int partition_size : data["partition_sizes"]) {
            iparams["partition_size"] = partition_size;
            iparams["system_size"] = system_size;

            for (auto json_fparams : data["fparams"]) {
                fparams["mzr_prob"] = json_fparams["mzr_prob"];
                for (int i = 0; i < num_runs; i++) {
                    configs.push_back(new QuantumAutomatonConfig(iparams, fparams));
                }
            }
        }
    }

    return configs;
}

void QuantumAutomatonConfig::init_state() {
    simulator = new QuantumAutomatonSimulator(system_size, mzr_prob, simulator_type);
}

std::map<std::string, Sample> QuantumAutomatonConfig::take_samples() {
    std::map<std::string, Sample> sample;
    sample.emplace("entropy", spatially_averaged_entropy());
    return sample;
}

QuantumAutomatonConfig::QuantumAutomatonConfig(std::map<std::string, int> iparams, std::map<std::string, float> fparams)
    : TimeConfig(iparams, fparams), Entropy(iparams, fparams) {

    simulator_type = (CliffordType) iparams["simulator_type"];
	mzr_prob = fparams["mzr_prob"];
}

void QuantumAutomatonConfig::timesteps(uint num_steps) {
    simulator->timesteps(num_steps);
}

std::map<std::string, int> QuantumAutomatonConfig::get_iparams() const {
	std::map<std::string, int> iparams;
    
    iparams["system_size"] = system_size;
    iparams["partition_size"] = partition_size;
    iparams["equilibration_steps"] = equilibration_steps;
    iparams["sampling_timesteps"] = sampling_timesteps;
    iparams["measurement_freq"] = measurement_freq;
    iparams["simulator_type"] = simulator_type;
    iparams["spacing"] = spacing;

	return iparams;
}

std::map<std::string, float> QuantumAutomatonConfig::get_fparams() const {
	std::map<std::string, float> fparams;
	fparams["mzr_prob"] = mzr_prob;

	return fparams;
}
