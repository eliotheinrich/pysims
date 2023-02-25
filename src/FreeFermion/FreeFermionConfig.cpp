#include "FreeFermionConfig.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream> 

using json = nlohmann::json;

std::vector<FreeFermionConfig*> FreeFermionConfig::load_json(std::string filename) {
    std::ifstream f(filename);
    
    json data = json::parse(f);

    uint num_configs = data["system_sizes"].size()*data["partition_sizes"].size()*data["fparams"].size();
    assert(num_configs > 0);

    std::map<std::string, int> iparams;
    std::map<std::string, float> fparams;

    std::vector<FreeFermionConfig*> configs(0);
    uint num_runs = data.value("num_runs", DEFAULT_NUM_RUNS);
    iparams["equilibration_steps"] = data.value("equilibration_steps", DEFAULT_EQUILIBRATION_STEPS);
    iparams["timesteps"] = data.value("timesteps", DEFAULT_TIMESTEPS);
    iparams["measurement_freq"] = data.value("measurement_freq", DEFAULT_MEASUREMENT_FREQ);
    iparams["spacing"] = data.value("spacing", DEFAULT_SPACING);

    for (int system_size : data["system_sizes"]) {
        for (int partition_size : data["partition_sizes"]) {
            iparams["partition_size"] = partition_size;
            iparams["system_size"] = system_size;
            for (auto json_fparams : data["fparams"]) {
                fparams["p1"] = json_fparams["p1"];
                fparams["p2"] = json_fparams["p2"];
                fparams["beta"] = json_fparams["beta"];
                fparams["filling_fraction"] = json_fparams["filling_fraction"];
                for (int i = 0; i < num_runs; i++) {
                    configs.push_back(new FreeFermionConfig(iparams, fparams));
                }
            }
        }
    }

    return configs;
}

FreeFermionConfig::FreeFermionConfig(std::map<std::string, int> iparams, std::map<std::string, float> fparams) {
    system_size = iparams["system_size"];

    equilibration_steps = iparams["equilibration_steps"];
    timesteps = iparams["timesteps"];
    measurement_freq = iparams["measurement_freq"];

    partition_size = iparams["partition_size"];
    spacing = iparams["spacing"];
    
    p1 = fparams["p1"];
    p2 = fparams["p2"];
    beta = fparams["beta"];
    filling_fraction = fparams["filling_fraction"];
}

std::map<std::string, int> FreeFermionConfig::get_iparams() const {
    std::map<std::string, int> iparams;
    iparams["system_size"] = system_size;
    iparams["partition_size"] = partition_size;
    iparams["equilibration_steps"] = equilibration_steps;
    iparams["timesteps"] = timesteps;
    iparams["measurement_freq"] = measurement_freq;

    return iparams;
}

std::map<std::string, float> FreeFermionConfig::get_fparams() const {
    std::map<std::string, float> fparams;
    fparams["p1"] = p1;
    fparams["p2"] = p2;
    fparams["beta"] =  beta;
    fparams["filling_fraction"] = filling_fraction;

    return fparams;
}

void FreeFermionConfig::compute(DataSlide* slide) {
    simulator = FreeFermionSimulator(system_size, p1, p2, beta, filling_fraction);

    simulator.timesteps(equilibration_steps);

    int num_timesteps, num_intervals;
    if (timesteps == 0) {
        num_timesteps = 0;
        num_intervals = 1;
    } else {
        num_timesteps = measurement_freq;
        num_intervals = timesteps/measurement_freq;
    }


    std::vector<Sample> entropy;
    for (int t = 0; t < num_intervals; t++) {
        simulator.timesteps(num_timesteps);
        Sample entropy_sample = Sample(simulator.spatially_averaged_entropy(system_size, partition_size, spacing));
        entropy.push_back(entropy_sample);
    }

    slide->add_data("entropy");
    slide->push_data("entropy", Sample::collapse(entropy));
}