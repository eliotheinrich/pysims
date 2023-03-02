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
    /*

    uint system_size = 4;
    QuantumGraphState state1(system_size);
    QuantumCHPState state2(system_size);
    uint num_gates = 1000;
    for (uint i = 0; i < num_gates; i++) {
        uint r = rand() % 7;
        uint q1 = rand() % system_size;
        uint q2 = rand() % system_size;
        while (q2 == q1) q2 = rand() % system_size;
        if (r == 0) {
            std::cout << "h " << q1 << std::endl;
            state1.h_gate(q1);
            state2.h_gate(q1);
        } else if (r == 1) {
            std::cout << "s " << q1 << std::endl;
            state1.s_gate(q1);
            state2.s_gate(q1);
        } else if (r == 2) {
            std::cout << "x " << q1 << std::endl;
            state1.x_gate(q1);
            state2.x_gate(q1);
        } else if (r == 3) {
            std::cout << "y " << q1 << std::endl;
            state1.y_gate(q1);
            state2.y_gate(q1);
        } else if (r == 4) {
            std::cout << "z " << q1 << std::endl;
            state1.z_gate(q1);
            state2.z_gate(q1);
        } else if (r == 5) {
            std::cout << "cz " << q1 << " " << q2 << std::endl;
            state1.cz_gate(q1, q2);
            state2.cz_gate(q1, q2);
        } else if (r == 6) {
            std::cout << "mzr " << q1 << " (" << q2 % 2 << ")\n";
            state1.mzr_forced(q1, q2 % 2);
            state2.mzr_forced(q1, q2 % 2);
        }

        uint num_qubits = rand() % system_size;
        std::vector<uint> qubits;
        for (uint j = 0; j < num_qubits; j++) {
            uint q = rand() % system_size;
            if (!count(qubits.begin(), qubits.end(), q)) qubits.push_back(q);
        }

        cout << i << " " << state1.entropy(qubits) << " " << state2.entropy(qubits) << std::endl;
        std::cout << "state1: \n" << state1.to_string() << std::endl;
        //std::cout << "state2: \n" << state2.to_string() << std::endl;
        print_vector(qubits);
        assert(state1.entropy(qubits) == state2.entropy(qubits));
    }
    */
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
    auto params = Params::load_json(data);
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