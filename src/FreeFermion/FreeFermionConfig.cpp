#include "FreeFermionConfig.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream> 

using json = nlohmann::json;

std::vector<FreeFermionConfig*> FreeFermionConfig::load_json(std::string filename) {
    std::ifstream f(filename);
    
    json data = json::parse(f);

    uint num_system_sizes = data["system_sizes"].size();
    uint num_partition_sizes = data["partition_sizes"].size();
    uint num_params = data["fparams"].size();
    uint num_configs = data["system_sizes"].size()*data["partition_sizes"].size()*data["fparams"].size();
    assert(num_configs > 0);


    std::vector<FreeFermionConfig*> configs(0);

    Params params;
    // Requirements for TimeConfig
    params.set("equilibration_timesteps", (int) data.value("equilibration_steps", DEFAULT_EQUILIBRATION_STEPS));
    params.set("sampling_timesteps", (int) data.value("sampling_timesteps", DEFAULT_SAMPLING_TIMESTEPS));
    params.set("measurement_freq", (int) data.value("measurement_freq", DEFAULT_MEASUREMENT_FREQ));
    params.set("temporal_avg", (int) data.value("temporal_avg", DEFAULT_TEMPORAL_AVG));
    params.set("num_runs", (int) data.value("num_runs", DEFAULT_NUM_RUNS));

    params.set("spacing", data.value("spacing", DEFAULT_SPACING));

    for (int system_size : data["system_sizes"]) {
        for (int partition_size : data["partition_sizes"]) {
            params.set("partition_size", partition_size);
            params.set("system_size", system_size);
            for (auto json_fparams : data["fparams"]) {
                params.set("p1", json_fparams["p1"]);
                params.set("p2", json_fparams["p2"]);
                params.set("beta", json_fparams["beta"]);
                params.set("filling_fraction", json_fparams["filling_fraction"]);
                configs.push_back(new FreeFermionConfig(params));
            }
        }
    }

    return configs;
}

FreeFermionConfig::FreeFermionConfig(Params &p) : TimeConfig(p), Entropy(p) { 
    p1 = fparams["p1"];
    p2 = fparams["p2"];
    beta = fparams["beta"];
    filling_fraction = fparams["filling_fraction"];
}

std::map<std::string, Sample> FreeFermionConfig::take_samples() {
    std::map<std::string, Sample> sample;
    sample.emplace("entropy", spatially_averaged_entropy());
    return sample;
}

void init_state() {
    simulator = new FreeFermionSimulator(system_size, p1, p2, beta, filling_fraction);
}

void timesteps(uint num_timesteps) {
    simulator->timesteps(num_timesteps);
}
