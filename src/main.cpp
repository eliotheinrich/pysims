#include "utils.cpp"

#include <iostream>
#include <memory>
#include <unistd.h>
#include <tuple>
#include <Eigen/Dense>

#ifdef SERIAL
#define DEFAULT_THREADS_PER_TASK Eigen::nbThreads()
#else
#define DEFAULT_THREADS_PER_TASK 1
#endif


using json = nlohmann::json;

void defaultf() {
    std::cout << "Default behavior\n";
}

std::tuple<std::string, uint32_t, uint32_t> parse_args(int argc, char *argv[]) {
    std::string filename;
    uint32_t num_threads;
    uint32_t num_threads_per_task;

    if (argc < 2) std::cout << "Must provide a config.\n";
    filename = argv[1];

    // Do this before assigning filename and num threads to explain segfault if arguments are incorrect.
    if (argc < 3) std::cout << "Must provide a number of threads.\n";

    if (argc == 3)
        num_threads_per_task = DEFAULT_THREADS_PER_TASK;
    else if (argc == 4)
        num_threads_per_task = std::stoi(argv[3]);
    else {
        std::cout << "Too many arguments. Must provide a config file, number of threads, and an optional number of threads per config.\n";
        num_threads_per_task = DEFAULT_THREADS_PER_TASK; // Assign to avoid compiler warning
        assert(false);
    }
    num_threads = std::stoi(argv[2]);

    return std::make_tuple(filename, num_threads, num_threads_per_task);
}

bool file_valid(std::string filename) {
    uint32_t strlen = filename.length();
    if (strlen < 6) { return false; }
    
    std::string extension = filename.substr(strlen - 5, strlen);
    std::string json_ext = ".json";
    if (extension != json_ext) { return false; }

    std::ifstream f(filename);

    return f.good();
}

int main(int argc, char *argv[]) {
    auto [filename, num_threads, num_threads_per_task] = parse_args(argc, argv);
    Eigen::setNbThreads(num_threads_per_task);

    std::cout << "Config: " << filename << "\n";
    std::cout << "Num threads: " << num_threads << "\n";
    std::cout << "Threads per config: " << num_threads_per_task << "\n";

    if (argc == 1) {
        defaultf();
        return 1;
    }

    bool valid = file_valid(filename);
    if (!valid) {
        std::cout << "Cannot find " << filename << "; aborting.\n";
        return 1;
    }

    std::ifstream f(filename);
    json data = json::parse(f);

    std::cout << "Starting job\n";

    std::string data_filename = data["filename"];

    Params metaparams;
    metaparams["num_threads"] = (int) num_threads;
    metaparams["num_threads_per_task"] = (int) num_threads_per_task;
    //metaparams["average_congruent_runs"] = 1;
    //metaparams["atol"] = 1e-10;
    //metaparams["rtol"] = 1e-10;

    ParallelCompute pc = build_pc(metaparams, data.dump());
    pc.compute(true);

    std::string data_prefix = "../data";
    pc.write_json(data_prefix + "/" + data_filename);
} 