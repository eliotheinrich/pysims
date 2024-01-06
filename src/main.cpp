#include "Models.h"

#include <Frame.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <Eigen/Dense>


using namespace dataframe;
using namespace dataframe::utils;

std::string load_file(const std::string& filename) {
  std::ifstream file(filename);

  std::string content;
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      content += line;
    }

    file.close();
  }

  return content;
}

void write_file(const std::string& filename, const std::string& content) {
  // Open the file with ofstream in binary mode to handle both text and binary files
  std::ofstream file(filename, std::ios::binary);

  if (file.is_open()) {
      // Write content to the file
      file.write(content.c_str(), static_cast<std::streamsize>(content.length()));

      // Check for any writing errors
      if (file.fail()) {
          std::cerr << "Error writing to file: " << filename << std::endl;
      } else {
          std::cout << "File written successfully: " << filename << std::endl;
      }

      // Close the file
      file.close();
  } else {
      std::cerr << "Error opening file: " << filename << std::endl;
  }
}

//
//std::vector<Params> load_config_file(const std::string& filename) {
//  return parse_config(load_file(filename));
//}
//
//template <class T>
//DataSlide execute_simulation(Params& params, uint32_t num_threads) {
//  TimeSamplingDriver<T> sim(params);
//  DataSlide slide = sim.generate_dataslide(num_threads);
//  return slide;
//}
//
//void simulation(int argc, char* argv[]) {
//  if (argc != 3) {
//    throw std::invalid_argument("Must provide a number of threads and config filename.");
//  }
//
//  int num_threads = std::stoi(argv[2]);
//  std::string filename = argv[1];
//  auto params = load_config_file(filename);
//  
//  for (auto param : params) {
//    std::string circuit_type = get<std::string>(param, "circuit_type");
//    DataSlide slide;
//    if (circuit_type == "random_clifford") {
//      slide = execute_simulation<RandomCliffordSimulator>(param, num_threads);
//    } else if (circuit_type == "quantum_automaton") {
//      slide = execute_simulation<QuantumAutomatonSimulator>(param, num_threads);
//    } else if (circuit_type == "sandpile_clifford") {
//      slide = execute_simulation<SandpileCliffordSimulator>(param, num_threads);
//    } else if (circuit_type == "self_organized_clifford") {
//      slide = execute_simulation<SelfOrganizedCliffordSimulator>(param, num_threads);
//    } else if (circuit_type == "post_selection_clifford") {
//      slide = execute_simulation<PostSelectionCliffordSimulator>(param, num_threads);
//    } else if (circuit_type == "phaseless") {
//      slide = execute_simulation<PhaselessSimulator>(param, num_threads);
//    } else if (circuit_type == "network_clifford") {
//      slide = execute_simulation<NetworkCliffordSimulator>(param, num_threads);
//    } else if (circuit_type == "environment") {
//      slide = execute_simulation<EnvironmentSimulator>(param, num_threads);
//    } else if (circuit_type == "mincut") {
//      slide = execute_simulation<MinCutSimulator>(param, num_threads);
//    } else if (circuit_type == "graph_clifford") {
//      slide = execute_simulation<GraphCliffordSimulator>(param, num_threads);
//    } else if (circuit_type == "random_circuit_sampling") {
//      slide = execute_simulation<RandomCircuitSamplingSimulator>(param, num_threads);
//    } else if (circuit_type == "grover_projection") {
//      slide = execute_simulation<GroverProjectionSimulator>(param, num_threads);
//    } else if (circuit_type == "grover_sat") {
//      slide = execute_simulation<GroverSATSimulator>(param, num_threads);
//    } else if (circuit_type == "brickwork_circuit") {
//      slide = execute_simulation<BrickworkCircuitSimulator>(param, num_threads);
//    } else if (circuit_type == "vqse") {
//        VQSEConfig vqse(param);
//        slide = vqse.compute(num_threads);
//    } else if (circuit_type == "vqse_circuit") {
//        VQSECircuitConfig vqse(param);
//        slide = vqse.compute(num_threads);
//    } else {
//      throw std::invalid_argument("Invalid circuit type passed.");
//    }
//  }
//}

#include <chrono>

#define TIMEIT(A) \
start = std::chrono::system_clock::now();         \
A;                                                \
end = std::chrono::system_clock::now();           \
elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000.0;                            \
std::cout << "Elapsed: " << elapsed << std::endl;

struct to_string {
  std::string operator()(int i) { return std::to_string(i); }
  std::string operator()(std::string_view s) { return std::string(s); }
};

void df_write_binary(const DataFrame& frame, const std::string& filename) {
  std::ofstream file(filename, std::ios::out | std::ios::binary);

  std::string buffer = glz::write_binary(frame);

  file << buffer;
  file.close();
}

void read_dataframe(int argc, char* argv[]) {
  std::string filename = argv[1];
  std::string content = load_file(filename);

  auto start = std::chrono::system_clock::now();         
  auto end = std::chrono::system_clock::now();           
  auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000.0;                            

  DataFrame frame;
  TIMEIT(frame = DataFrame(content));
  //std::string s = glz::write_json(frame);
  //TIMEIT(frame = DataFrame(content));

  //glz::write_file(frame, "./frame.json", )
  df_write_binary(frame, "frame.txt");

  //write_file("new_frame2.json", s);

}

int main(int argc, char* argv[]) {
  read_dataframe(argc, argv);
}
