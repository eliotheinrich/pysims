#include "QuantumAutomatonConfig.h"
#include "RandomCliffordConfig.h"
#include "MinCutConfig.h"


#include <nlohmann/json.hpp>
#include <iostream>

using namespace std;
using json = nlohmann::json;


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

    string filename = argv[1];
    uint num_threads = std::stoi(argv[2]);
    bool valid = file_valid(filename);
    if (!valid) {
        cout << "Cannot find " << filename << "; aborting.\n";
        return 1;
    }

    std::ifstream f(filename);
    json data = json::parse(f);
    std::string simulator_type = data["circuit_type"];

    std::cout << "Starting job\n";

    std::string data_prefix = "../data/";
    if (simulator_type == "quantum_automaton") {
        auto configs = QuantumAutomatonConfig::load_json(data);
        std::string data_filename = data["filename"];
        ParallelCompute pc(configs);
        DataFrame df = pc.compute(num_threads);
        df.write_json(data_prefix + data_filename);
    } else if (simulator_type == "random_clifford") {
        auto configs = RandomCliffordConfig::load_json(data);
        std::string data_filename = data["filename"];
        ParallelCompute pc(configs);
        DataFrame df = pc.compute(num_threads);
        df.write_json(data_prefix + data_filename);
    } else if (simulator_type == "mincut") {
        auto configs = MinCutConfig::load_json(data);
        std::string data_filename = data["filename"];
        ParallelCompute pc(configs);
        DataFrame df = pc.compute(num_threads);
        df.write_json(data_prefix + data_filename);
    } else {
        defaultf();
    }
    std::cout << "Finishing job\n";
}