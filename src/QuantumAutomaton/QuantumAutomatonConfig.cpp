#include "QuantumAutomatonConfig.h"
#include "Simulator.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<QuantumAutomatonConfig*> QuantumAutomatonConfig::load_json(json data) {
	uint num_system_sizes = data["system_sizes"].size();
	uint num_partition_sizes = data["partition_sizes"].size();
	uint num_params = data["fparams"].size();
	uint num_configs = num_system_sizes*num_partition_sizes*num_params;
	assert(num_configs > 0);

    std::vector<QuantumAutomatonConfig*> configs;

    Params params;
    // Requirements for TimeConfig
    params.set("equilibration_timesteps", (int) data.value("equilibration_timesteps", DEFAULT_EQUILIBRATION_STEPS));
    params.set("sampling_timesteps", (int) data.value("sampling_timesteps", DEFAULT_SAMPLING_TIMESTEPS));
    params.set("measurement_freq", (int) data.value("measurement_freq", DEFAULT_MEASUREMENT_FREQ));
    params.set("temporal_avg", (int) data.value("temporal_avg", DEFAULT_TEMPORAL_AVG));
    params.set("num_runs", (int) data.value("num_runs", DEFAULT_NUM_RUNS));
    params.set("spacing", (int) data.value("spacing", DEFAULT_SPACING));


    params.set("clifford_state", (int) parse_clifford_type(data.value("clifford_state", DEFAULT_CLIFFORD_STATE)));
    
    for (int system_size : data["system_sizes"]) {
        for (int partition_size : data["partition_sizes"]) {
            params.set("partition_size", partition_size);
            params.set("system_size", system_size);

            for (auto json_fparams : data["fparams"]) {
                params.set("mzr_prob", (float) json_fparams["mzr_prob"]);
                configs.push_back(new QuantumAutomatonConfig(params));
            }
        }
    }

    return configs;
}

void QuantumAutomatonConfig::init_state() {
    simulator = new QuantumAutomatonSimulator(system_size, mzr_prob, simulator_type);
}

QuantumAutomatonConfig::QuantumAutomatonConfig(Params &params) : EntropyConfig(params) {
    simulator_type = (CliffordType) params.geti("clifford_state");
	mzr_prob = params.getf("mzr_prob");
}

void QuantumAutomatonConfig::timesteps(uint num_steps) {
    simulator->timesteps(num_steps);
}
