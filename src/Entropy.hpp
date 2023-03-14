#ifndef SIM_H
#define SIM_H

#include <iostream>
#include <DataFrame.hpp>
#include <Simulator.hpp>
#include <algorithm>
#include <numeric>
#include <math.h>

template <typename T>
static void print_vector(std::vector<T> v) {
    std::cout << "[";
    for (auto const &t : v) {
        std::cout << t << ", ";
    } std::cout << "]\n";
}

class Entropy {
    public:
        uint system_size;
        uint partition_size;
        uint spacing;

        Entropy() {}

        Entropy(Params &params) {
            system_size = params.geti("system_size");
            partition_size = params.geti("partition_size");
            spacing = params.geti("spacing", DEFAULT_SPACING);
        }

        virtual float entropy(std::vector<uint> &sites) const=0;

        Sample spatially_averaged_entropy(uint system_size, uint partition_size, uint spacing) const {
            std::vector<uint> sites(partition_size);
            std::iota(sites.begin(), sites.end(), 0);

            uint num_partitions = std::max((system_size - partition_size)/spacing, 1u);

            float s = 0.;
            float s2 = 0.;

            std::vector<uint> offset_sites(partition_size);
            for (uint i = 0; i < num_partitions; i++) { // TODO test
                std::transform(sites.begin(), 
                               sites.end(),
                               offset_sites.begin(), 
                               [i, spacing, system_size](uint x) { return (x + i*spacing) % system_size; });

                float st = entropy(offset_sites);
                s += st;
                s2 += st*st;
            }

            s /= num_partitions;
            s2 /= num_partitions;

            float stdev = std::sqrt(std::abs(s2 - s*s));
            
            return Sample(s, stdev, num_partitions);        
        }

        Sample spatially_averaged_entropy() const {
            return spatially_averaged_entropy(system_size, partition_size, spacing);
        }

};

class EntropySimulator : public Simulator, public Entropy {
    public:
        EntropySimulator(Params &params) : Simulator(params), Entropy(params) {}

        virtual std::map<std::string, Sample> take_samples() {
            std::map<std::string, Sample> sample;
            sample.emplace("entropy", spatially_averaged_entropy());
            return sample;
        }
};

class MutualInformationSimulator : public Simulator, public Entropy {
    private:
        uint num_bins;
        float min_eta;
        float max_eta;

        std::vector<uint> to_interval(uint x1, uint x2) {
            assert(x1 < system_size);
            assert(x2 < system_size);
            std::vector<uint> interval;
            uint i = x1;
            while (true) {
                interval.push_back(i);
                i = (i + 1) % system_size;
                if (i == x2) {
                    interval.push_back(i);
                    return interval;
                }
            }
        }

        std::vector<uint> to_combined_interval(int x1, int x2, int x3, int x4) {
            std::vector<uint> interval1 = to_interval(x1, x2);
            std::sort(interval1.begin(), interval1.end());
            std::vector<uint> interval2 = to_interval(x3, x4);
            std::sort(interval1.begin(), interval2.end());
            std::vector<uint> combined_interval;
            std::set_union(interval1.begin(), interval1.end(), interval2.begin(), interval2.end(), combined_interval.begin());

            return combined_interval;
        }

        float x(int x1, int x2) {
            return std::sin(std::abs(x1 - x2)*M_PI/system_size);
        }

        float eta(int x1, int x2, int x3, int x4) {
            float x12 = x(x1, x2);
            float x34 = x(x3, x4);
            float x13 = x(x1, x3);
            float x24 = x(x2, x4);
            return x12*x34/(x13*x24);
        }

        float bin_width() {
            return (max_eta - min_eta)/num_bins;
        }

        std::vector<float> bins() {
            float h = bin_width();
            std::vector<float> bin;
            for (uint i = 0; i < num_bins; i++) bin.push_back(i*h + min_eta);

            return bin;
        }

        std::vector<float> compute_entropy_table() {
            std::vector<float> table;

            for (uint x1 = 0; x1 < system_size; x1++) {
                uint x2 = (x1 + partition_size) % system_size;

                std::vector<uint> sites = to_interval(x1, x2);
                table.push_back(entropy(sites));
            }

            return table;
        }

    public:
        MutualInformationSimulator(Params &params) : Simulator(params), Entropy(params) {
            num_bins = params.geti("num_bins");
            min_eta = params.getf("min_eta");
            max_eta = params.getf("max_eta");
        }

        virtual std::map<std::string, Sample> take_samples() {
            std::map<std::string, Sample> sample;

            std::vector<float> entropy_table = compute_entropy_table();
            for (uint x1 = 0; x1 < system_size; x1++) {
                for (uint x3 = 0; x3 < system_size; x3++) {
                    uint x2 = (x1 + partition_size) % system_size;
                    uint x4 = (x3 + partition_size) % system_size;

                    std::vector<uint> combined_sites = to_combined_interval(x1, x2, x3, x4);

                    float entropy1 = entropy_table[x1];
                    float entropy2 = entropy_table[x3];
                    float entropy3 = entropy(combined_sites);
                    Sample I = Sample(entropy1 + entropy2 - entropy3);

                    std::string key = "I_" + std::to_string(x1) + "_" + std::to_string(x3);
                    if (sample.count(key)) { 
                        sample[key] = Sample(I);
                    } else {
                        sample[key] = sample[key].combine(I);
                    }
                }
            }

            return sample;
        }

};

// TODO find a better place for this to live
static inline const uint mod(int a, int b) {
	int c = a % b;
	return (c < 0) ? c + b : c;
}


#endif