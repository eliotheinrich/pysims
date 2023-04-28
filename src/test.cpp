#include <iostream>
#include <Tableau.h>
#include <DataFrame.hpp>
#include <random>

class ReductionCircuitConfig : public Config {
    private:
        uint system_size;
        uint num_runs;
        uint num_strings;

    public:
        ReductionCircuitConfig(Params &params) : Config(params) {
            this->system_size = params.get<int>("system_size");
            this->num_runs = params.get<int>("num_runs");
            this->num_strings = params.get<int>("num_strings");
        }

        virtual uint get_nruns() const override {
            return num_runs;
        }

        virtual DataSlide compute() override {
            DataSlide ds;

            ds.add<int>("system_size", system_size);
            ds.add_data("circuit_depth");
            std::vector<uint> circuit_depth;

            std::minstd_rand *r = new std::minstd_rand();
            for (uint i = 0; i < num_strings; i++) {
                PauliString p1 = PauliString::rand(system_size, r);
                PauliString p2 = PauliString::rand(system_size, r);

                PauliString p3 = p1.copy();

                Circuit circuit = p1.transform(p2);
                apply_circuit(circuit, p3);
                assert(p2 == p3);

                circuit_depth.push_back(circuit.size());
            }

            for (auto const &d : circuit_depth) ds.push_data("circuit_depth", d);

            return ds;
        }

        virtual std::unique_ptr<Config> clone() override {
            std::unique_ptr<ReductionCircuitConfig> config(new ReductionCircuitConfig(params));
            return config;
        }
};


int main() {
    uint num_strings = 100;
    uint num_runs = 1;
    std::vector<std::unique_ptr<Config>> cfgs;
    std::vector<uint> system_sizes{5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60};
    for (auto const &s : system_sizes) {
        Params p;
        p.add<int>("system_size", s);
        p.add<int>("num_runs", num_runs);
        p.add<int>("num_strings", num_strings);
        cfgs.push_back(std::unique_ptr<Config>(new ReductionCircuitConfig(p)));
    }
    ParallelCompute pc(std::move(cfgs));

    DataFrame df = pc.compute(4, true);
    df.write_json("test_data.json", true);
}