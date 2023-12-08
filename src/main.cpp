#include "Models.h"

#include <Frame.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <Eigen/Dense>


using namespace dataframe;
using namespace dataframe::utils;

std::vector<Params> load_config_file(const std::string& filename) {
  std::ifstream file(filename);

  std::string content;
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      content += line;
    }

    file.close();
  }

  return parse_config(content);
}

template <class T>
DataSlide execute_simulation(Params& params, uint32_t num_threads) {
  TimeSamplingDriver<T> sim(params);
  DataSlide slide = sim.generate_dataslide(num_threads);
  return slide;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    throw std::invalid_argument("Must provide a number of threads and config filename.");
  }

  int num_threads = std::stoi(argv[2]);
  std::string filename = argv[1];
  auto params = load_config_file(filename);
  
  for (auto param : params) {
    std::string circuit_type = get<std::string>(param, "circuit_type");
    DataSlide slide;
    if (circuit_type == "random_clifford") {
      slide = execute_simulation<RandomCliffordSimulator>(param, num_threads);
    } else if (circuit_type == "quantum_automaton") {
      slide = execute_simulation<QuantumAutomatonSimulator>(param, num_threads);
    } else if (circuit_type == "sandpile_clifford") {
      slide = execute_simulation<SandpileCliffordSimulator>(param, num_threads);
    } else if (circuit_type == "self_organized_clifford") {
      slide = execute_simulation<SelfOrganizedCliffordSimulator>(param, num_threads);
    } else if (circuit_type == "post_selection_clifford") {
      slide = execute_simulation<PostSelectionCliffordSimulator>(param, num_threads);
    } else if (circuit_type == "phaseless") {
      slide = execute_simulation<PhaselessSimulator>(param, num_threads);
    } else if (circuit_type == "network_clifford") {
      slide = execute_simulation<NetworkCliffordSimulator>(param, num_threads);
    } else if (circuit_type == "environment") {
      slide = execute_simulation<EnvironmentSimulator>(param, num_threads);
    } else if (circuit_type == "mincut") {
      slide = execute_simulation<MinCutSimulator>(param, num_threads);
    } else if (circuit_type == "graph_clifford") {
      slide = execute_simulation<GraphCliffordSimulator>(param, num_threads);
    } else if (circuit_type == "random_circuit_sampling") {
      slide = execute_simulation<RandomCircuitSamplingSimulator>(param, num_threads);
    } else if (circuit_type == "grover_projection") {
      slide = execute_simulation<GroverProjectionSimulator>(param, num_threads);
    } else if (circuit_type == "grover_sat") {
      slide = execute_simulation<GroverSATSimulator>(param, num_threads);
    } else if (circuit_type == "brickwork_circuit") {
      slide = execute_simulation<BrickworkCircuitSimulator>(param, num_threads);
    } else if (circuit_type == "vqse") {
        VQSEConfig vqse(param);
        slide = vqse.compute(num_threads);
    } else if (circuit_type == "vqse_circuit") {
        VQSECircuitConfig vqse(param);
        slide = vqse.compute(num_threads);
    } else {
      throw std::invalid_argument("Invalid circuit type passed.");
    }
  }

}
