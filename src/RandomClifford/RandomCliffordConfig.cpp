#include "RandomCliffordConfig.h"
#include "UndrivenRandomCliffordSimulator.hpp"
#include "DrivenRandomCliffordSimulator.hpp"
#include "Simulator.hpp"
#include <assert.h>

std::vector<RandomCliffordConfig*> RandomCliffordConfig::load_json(nlohmann::json data) {
	uint num_system_sizes = data["system_sizes"].size();
	uint num_partition_sizes = data["partition_sizes"].size();
	uint num_fparams = data["fparams"].size();
    uint num_iparams = data["iparams"].size();
	uint num_configs = num_system_sizes*num_partition_sizes*num_iparams;
	assert(num_configs > 0);
    assert(num_fparams == num_iparams);


    std::vector<RandomCliffordConfig*> configs;

    Params params;
    // Requirements for TimeConfig
    params.set("equilibration_timesteps", (int) data.value("equilibration_timesteps", DEFAULT_EQUILIBRATION_STEPS));
    params.set("sampling_timesteps", (int) data.value("sampling_timesteps", DEFAULT_SAMPLING_TIMESTEPS));
    params.set("measurement_freq", (int) data.value("measurement_freq", DEFAULT_MEASUREMENT_FREQ));
    params.set("temporal_avg", (int) data.value("temporal_avg", DEFAULT_TEMPORAL_AVG));
    params.set("num_runs", (int) data.value("num_runs", DEFAULT_NUM_RUNS));

    params.set("spacing", (int) data.value("spacing", DEFAULT_SPACING));
    params.set("clifford_state", (int) parse_clifford_type(data.value("clifford_state", DEFAULT_CLIFFORD_STATE)));
    params.set("drive_type", (int) parse_drive_type(data.value("drive_type", DEFAULT_DRIVE_TYPE)));
    for (int system_size : data["system_sizes"]) {
        for (int partition_size : data["partition_sizes"]) {
            params.set("partition_size", partition_size);
            params.set("system_size", system_size);
            
            for (uint n = 0; n < num_fparams; n++) {
                auto json_fparams = data["fparams"][n];
                auto json_iparams = data["iparams"][n];
                params.set("mzr_prob", (float) json_fparams["mzr_prob"]);
                params.set("gate_width", (int) json_iparams["gate_width"]);
                configs.push_back(new RandomCliffordConfig(params));
            }
        }
    }

    return configs;
}

void RandomCliffordConfig::init_state() {
    switch (drive_type) {
        case DriveType::Undriven : simulator = new UndrivenRandomCliffordSimulator(system_size, init_mzr_prob, gate_width, clifford_state);
        case DriveType::Test : simulator = new DrivenRandomCliffordSimulator(system_size, init_mzr_prob, gate_width, clifford_state);
    }
}

void RandomCliffordConfig::timesteps(uint num_steps) {
    simulator->timesteps(num_steps);
}

std::map<std::string, Sample> RandomCliffordConfig::take_samples() {
    std::map<std::string, Sample> sample;
    sample.emplace("entropy", spatially_averaged_entropy());
    return sample;
}

RandomCliffordConfig::RandomCliffordConfig(Params &params) : TimeConfig(params), Entropy(params) {
    clifford_state = (CliffordType) params.geti("clifford_state");
    drive_type = (DriveType) params.geti("drive_type");
    gate_width = params.geti("gate_width");
	init_mzr_prob = params.getf("mzr_prob");
}
