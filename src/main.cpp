// Clifford simulators
#include <QuantumAutomatonSimulator.h>
#include <RandomCliffordSimulator.h>
#include <SandpileCliffordSimulator.h>
#include <SelfOrganizedCliffordSimulator.h>
#include <MinCutSimulator.h>
#include <BlockSimulator.h>
#include <GraphCliffordSimulator.h>

// Quantum simulators
#include <GroverProjectionSimulator.h>

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <memory>

#ifdef OMPI
#include <mpi.h>
#endif

using json = nlohmann::json;

void defaultf() {
    std::cout << "Default behavior\n";
}

bool file_valid(std::string filename) {
    uint strlen = filename.length();
    if (strlen < 6) { return false; }
    
    std::string extension = filename.substr(strlen - 5, strlen);
    std::string json_ext = ".json";
    if (extension != json_ext) { return false; }

    std::ifstream f(filename);

    return f.good();
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        defaultf();
        return 1;
    }

#ifdef OMPI
    MPI_Init(NULL, NULL);

    // if running with OpenMPI, number of processes is provided to mpirun and not as an argument to main
    if (argc != 2) {
        DO_IF_MASTER(std::cout << "Incorrect arguments.\n";)
    }
    uint num_threads = 0;
#else
    if (argv != 3) std::cout << "Incorrect arguments.\n";
    uint num_threads = std::stoi(argv[2]);
#endif

    std::string filename = argv[1];
    bool valid = file_valid(filename);
    if (!valid) {
        DO_IF_MASTER(std::cout << "Cannot find " << filename << "; aborting.\n";)
        return 1;
    }

    std::ifstream f(filename);
    json data = json::parse(f);
    std::string circuit_type = data["circuit_type"];

    DO_IF_MASTER(std::cout << "Starting job\n";)

    std::string data_filename = data["filename"];
    auto params = Params::load_json(data, true);
    std::vector<std::unique_ptr<Config>> configs;

    std::string data_prefix = "../data/";
    for (auto param : params) {
        std::unique_ptr<TimeConfig> config(new TimeConfig(param));
        std::unique_ptr<Simulator> sim;

        if (circuit_type == "quantum_automaton") sim = std::unique_ptr<Simulator>(new QuantumAutomatonSimulator(param));
        else if (circuit_type == "random_clifford") sim = std::unique_ptr<Simulator>(new RandomCliffordSimulator(param));
        else if (circuit_type == "soc_clifford") sim = std::unique_ptr<Simulator>(new SelfOrganizedCliffordSimulator(param));
        else if (circuit_type == "sandpile_clifford") sim = std::unique_ptr<Simulator>(new SandpileCliffordSimulator(param));
        else if (circuit_type == "mincut") sim = std::unique_ptr<Simulator>(new MinCutSimulator(param));
        else if (circuit_type == "blocksim") sim = std::unique_ptr<Simulator>(new BlockSimulator(param));
        else if (circuit_type == "graphsim") sim = std::unique_ptr<Simulator>(new GraphCliffordSimulator(param));

        else if (circuit_type == "grover_projection") sim = std::unique_ptr<Simulator>(new GroverProjectionSimulator(param));
        else {
            defaultf();
            return 0;
        }

        config->init_simulator(std::move(sim));
        configs.push_back(std::move(config));
    }

    ParallelCompute pc(std::move(configs), num_threads);
    pc.compute(true);
    DO_IF_MASTER(
        pc.write_json(data_prefix + data_filename);
        std::cout << "Finishing job.\n";
    )

#ifdef OMPI
    MPI_Finalize();
#endif
} 