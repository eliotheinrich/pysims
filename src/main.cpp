#include "QuantumAutomatonSimulator.h"
#include "RandomCliffordSimulator.h"
#include "ClusterRandomCliffordSimulator.h"
#include "MinCutSimulator.h"

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

using namespace std;
using json = nlohmann::json;

#include "QuantumCHPState.h"
#include "QuantumGraphState.h"

void defaultf() {
    cout << "Default behavior\n";
}

bool file_valid(string filename) {
    uint strlen = filename.length();
    if (strlen < 6) { return false; }
    
    string extension = filename.substr(strlen - 5, strlen);
    string json_ext = ".json";
    if (extension != json_ext) { return false; }

    ifstream f(filename);

    return f.good();
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        defaultf();
        return 1;
    }

    if (argc != 3) cout << "Incorrect arguments.\n";

    string filename = argv[1];
    uint num_threads = std::stoi(argv[2]);
    bool valid = file_valid(filename);
    if (!valid) {
        cout << "Cannot find " << filename << "; aborting.\n";
        return 1;
    }

    std::ifstream f(filename);
    json data = json::parse(f);
    std::string circuit_type = data["circuit_type"];

    std::cout << "Starting job\n";

    std::string data_filename = data["filename"];
    auto params = Params::load_json(data, true);
    std::vector<Config*> configs;

    std::string data_prefix = "../data/";
    if (circuit_type == "quantum_automaton") {
        for (auto param : params) {
            TimeConfig *config = new TimeConfig(param);
            config->init_simulator(new QuantumAutomatonSimulator(param));
            configs.push_back(config);
        }
    } else if (circuit_type == "random_clifford") {
        for (auto param : params) {
            TimeConfig *config = new TimeConfig(param);
            config->init_simulator(new RandomCliffordSimulator(param));
            configs.push_back(config);
        }
    } else if (circuit_type == "soc_random_clifford") { 
        for (auto param : params) {
            TimeConfig *config = new TimeConfig(param);
            config->init_simulator(new ClusterRandomCliffordSimulator(param));
            configs.push_back(config);
        }
    } else if (circuit_type == "mincut") {
        for (auto param : params) {
            TimeConfig *config = new TimeConfig(param);
            config->init_simulator(new MinCutSimulator(param));
            configs.push_back(config);
        }
    } else {
        defaultf();
    }

    ParallelCompute pc(configs);
    DataFrame df = pc.compute(num_threads);
    df.write_json(data_prefix + data_filename);
    std::cout << "Finishing job\n";
}