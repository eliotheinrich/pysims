#pragma once

#include <iostream>
#include <sstream>
#include <DataFrame.hpp>
#include <Simulator.hpp>
#include <algorithm>
#include <numeric>
#include <math.h>

#define DEFAULT_RENYI_INDICES "2"
#define DEFAULT_SAMPLE_ENTROPY true
#define DEFAULT_SPATIALLY_AVERAGED_ENTROPY true

#define DEFAULT_SAMPLE_ALL_PARTITION_SIZES false

#define DEFAULT_SAMPLE_SURFACE_AVALANCHE false

#define DEFAULT_SAMPLE_MUTUAL_INFORMATION false
#define DEFAULT_NUM_MI_BINS 100
#define DEFAULT_MIN_ETA 0.01
#define DEFAULT_MAX_ETA 1.0

#define DEFAULT_SAMPLE_FIXED_MUTUAL_INFORMATION false

#define DEFAULT_SAMPLE_AVALANCHE_SIZE false

template <typename T>
static std::string print_vector(const std::vector<T> v) {
    std::string s = "";
    s += "[";
    for (auto const &t : v) {
        s += std::to_string(t) + ", ";
    } s += "]\n";
    return s;
}

class Entropy {
    protected:
        std::vector<uint32_t> to_interval(uint32_t x1, uint32_t x2) const {
            assert(x1 < system_size);
            assert(x2 <= system_size);
            if (x2 == system_size) x2 = 0;
            std::vector<uint32_t> interval;
            uint32_t i = x1;
            while (true) {
                interval.push_back(i);
                i = (i + 1) % system_size;
                if (i == x2) {
                    return interval;
                }
            }
        }

        std::vector<uint32_t> to_combined_interval(int x1, int x2, int x3, int x4) const {
            std::vector<uint32_t> interval1 = to_interval(x1, x2);
            std::sort(interval1.begin(), interval1.end());
            std::vector<uint32_t> interval2 = to_interval(x3, x4);
            std::sort(interval2.begin(), interval2.end());
            std::vector<uint32_t> combined_interval;


            std::set_union(interval1.begin(), interval1.end(), 
                           interval2.begin(), interval2.end(), 
                           std::back_inserter(combined_interval));

            return combined_interval;
        }

        double get_x(int x1, int x2) const {
            return std::sin(std::abs(x1 - x2)*M_PI/system_size);
        }

        double get_eta(int x1, int x2, int x3, int x4) const {
            double x12 = get_x(x1, x2);
            double x34 = get_x(x3, x4);
            double x13 = get_x(x1, x3);
            double x24 = get_x(x2, x4);
            return x12*x34/(x13*x24);
        }

		std::vector<double> compute_entropy_table(uint32_t index) const {
			std::vector<double> table;

			for (uint32_t x1 = 0; x1 < system_size; x1++) {
				uint32_t x2 = (x1 + partition_size) % system_size;

				std::vector<uint32_t> sites = to_interval(x1, x2);
				table.push_back(entropy(sites, index));
			}

			return table;
		}

    public:
        uint32_t system_size;
        uint32_t partition_size;
        uint32_t spacing;

        Entropy() {}

        Entropy(Params &params) {
            system_size = get<int>(params, "system_size");
            partition_size = get<int>(params, "partition_size", system_size/2); 
            spacing = get<int>(params, "spacing", DEFAULT_SPACING);
        }

        // This is the primary virtual function which must be overloaded by inheriting classes
        virtual double entropy(const std::vector<uint32_t> &sites, uint32_t index) const=0;

        double cum_entropy(uint32_t i, uint32_t index = 2u, bool direction = true) const {
            if (direction) { // Left-oriented cumulative entropy
                std::vector<uint32_t> sites(i+1);
                std::iota(sites.begin(), sites.end(), 0);
                return entropy(sites, index);
            } else { // Right-oriented cumulative entropy
                std::vector<uint32_t> sites(system_size - i);
                std::iota(sites.begin(), sites.end(), i);
                return entropy(sites, index);
            }
        }

        std::vector<double> get_entropy_surface(uint32_t index) const {
            std::vector<double> entropy_surface(system_size);

            for (uint32_t i = 0; i < system_size; i++)
                entropy_surface[i] = cum_entropy(i, index);

            return entropy_surface;
        }

        Sample spatially_averaged_entropy(uint32_t system_size, uint32_t partition_size, uint32_t spacing, uint32_t index) const {
            std::vector<uint32_t> sites(partition_size);
            std::iota(sites.begin(), sites.end(), 0);

            uint32_t num_partitions = std::max((system_size - partition_size)/spacing, 1u);

            double s = 0.;
            double s2 = 0.;

            std::vector<uint32_t> offset_sites(partition_size);
            for (uint32_t i = 0; i < num_partitions; i++) {
                std::transform(sites.begin(), 
                               sites.end(),
                               offset_sites.begin(), 
                               [i, spacing, system_size](uint32_t x) { return (x + i*spacing) % system_size; });

                double st = entropy(offset_sites, index);
                s += st;
                s2 += st*st;
            }

            s /= num_partitions;
            s2 /= num_partitions;

            double stdev = std::sqrt(std::abs(s2 - s*s));
            
            return Sample(s, stdev, num_partitions);        
        }

        Sample spatially_averaged_entropy(uint32_t index) const {
            return spatially_averaged_entropy(system_size, partition_size, spacing, index);
        }
};

class EntropySimulator : public Simulator, public Entropy {
    private:
        // Expects a list of indices in the format "1,2,3"
        static std::vector<uint32_t> parse_renyi_indices(const std::string &renyi_indices_str) {
            std::vector<uint32_t> indices;
            std::stringstream ss(renyi_indices_str);
            std::string token;

            while (std::getline(ss, token, ',')) {
                try {
                    uint32_t number = std::stoi(strip(token));
                    indices.push_back(number);
                } catch (const std::exception &e) {
                }
            }

            return indices;
        }

    protected:
        bool sample_entropy;
        bool spatially_average;
        std::vector<uint32_t> renyi_indices;

        bool sample_all_partition_sizes;

        bool sample_surface_avalanche;

        bool sample_mutual_information;
        uint32_t num_mi_bins;
        double min_eta;
        double max_eta;

        bool sample_fixed_mutual_information;
        uint32_t x1;
        uint32_t x2;
        uint32_t x3;
        uint32_t x4;

        bool sample_avalanche_size;
        bool start_sampling;
        uint32_t num_av_bins;
        double min_av;
        double max_av;
        std::map<uint32_t, std::vector<uint32_t>> avalanche_size_samples;
        bool surface_bit;
        std::vector<std::map<uint32_t, std::vector<double>>> entropy_surface;

        uint32_t get_bin_idx(double c, double a, double b, uint32_t num_bins) const {
            if ((c < a) || (c > b)) {
                std::cout << c << " is not between " << a << " and " << b << ". \n";
                assert(false);
            }
            double bin_width = (b - a)/num_bins;
            return uint32_t((c - a) / bin_width);
        }


    public:
        EntropySimulator(Params &params) : Simulator(params), Entropy(params) {
                                            
            sample_entropy = get<int>(params, "sample_entropy", DEFAULT_SAMPLE_ENTROPY);
            sample_all_partition_sizes = get<int>(params, "sample_all_partition_sizes", DEFAULT_SAMPLE_ALL_PARTITION_SIZES);
            spatially_average = get<int>(params, "spatial_avg", DEFAULT_SPATIALLY_AVERAGED_ENTROPY);

            sample_mutual_information = get<int>(params, "sample_mutual_information", DEFAULT_SAMPLE_MUTUAL_INFORMATION);
            if (sample_mutual_information) {
                assert(partition_size > 0);
                num_mi_bins = get<int>(params, "num_mi_bins", DEFAULT_NUM_MI_BINS);
                min_eta = get<double>(params, "min_eta", DEFAULT_MIN_ETA);
                max_eta = get<double>(params, "max_eta", DEFAULT_MAX_ETA);
            }

            sample_fixed_mutual_information = get<int>(params, "sample_fixed_mutual_information", DEFAULT_SAMPLE_FIXED_MUTUAL_INFORMATION);
            if (sample_fixed_mutual_information) {
                x1 = get<int>(params, "x1");
                x2 = get<int>(params, "x2");
                x3 = get<int>(params, "x3");
                x4 = get<int>(params, "x4");
            }

            renyi_indices = parse_renyi_indices(get<std::string>(params, "renyi_indices", DEFAULT_RENYI_INDICES));

            sample_avalanche_size = get<int>(params, "sample_avalanche_size", DEFAULT_SAMPLE_AVALANCHE_SIZE);
            if (sample_avalanche_size) {
                num_av_bins = get<int>(params, "num_av_bins", system_size);
                avalanche_size_samples = std::map<uint32_t, std::vector<uint32_t>>();
                for (auto const &i : renyi_indices)
                    avalanche_size_samples[i] = std::vector<uint32_t>(num_av_bins, 0u);
                
                entropy_surface = std::vector<std::map<uint32_t, std::vector<double>>>(2u);
                min_av = get<double>(params, "min_av_size", 0.0);
                max_av = get<double>(params, "max_av_size", system_size);
                surface_bit = true;
            }
        }

        void take_avalanche_samples() {
            if (!(sample_avalanche_size && start_sampling))
                return;

            for (auto const &i : renyi_indices) {
                if (surface_bit) {
                    entropy_surface[0][i] = get_entropy_surface(i);
                } else {
                    entropy_surface[1][i] = get_entropy_surface(i);

                    double s = 0.0;
                    for (uint32_t j = 0; j < system_size; j++)
                        s += std::abs(entropy_surface[1][i][j] - entropy_surface[0][i][j]);
                    
                    uint32_t idx = get_bin_idx(s, min_av, max_av, num_av_bins);
                    avalanche_size_samples[i][idx]++;
                }
            }
            
            surface_bit = !surface_bit;
        }

        void equilibration_timesteps(uint32_t num_steps) override {
            start_sampling = false;
            timesteps(num_steps);
            start_sampling = true;
        }

        void add_entropy_samples(data_t &samples, uint32_t index) const {
            Sample s = spatially_average ? spatially_averaged_entropy(system_size, partition_size, spacing, index) : cum_entropy(partition_size, index);
            samples.emplace("entropy" + std::to_string(index) + "_" + std::to_string(partition_size), s);
        }

        void add_mutual_information_samples(data_t &samples, uint32_t index) const {
            std::vector<double> entropy_table = compute_entropy_table(index);
            for (uint32_t x1 = 0; x1 < system_size; x1++) {
                for (uint32_t x3 = 0; x3 < system_size; x3++) {
                    uint32_t x2 = (x1 + partition_size) % system_size;
                    uint32_t x4 = (x3 + partition_size) % system_size;

                    double eta = get_eta(x1, x2, x3, x4);
                    if (!(eta > min_eta) || !(eta < max_eta)) continue;

                    std::vector<uint32_t> combined_sites = to_combined_interval(x1, x2, x3, x4);

                    double entropy1 = entropy_table[x1];
                    double entropy2 = entropy_table[x3];
                    double entropy3 = entropy(combined_sites, index);
                    
                    double mutual_information_sample = entropy1 + entropy2 - entropy3;

                    std::string key = "I" + std::to_string(index) + "_" + std::to_string(get_bin_idx(eta, min_eta, max_eta, num_mi_bins));
                    if (samples.count(key))
                        samples[key] = mutual_information_sample;
                    else
                        samples[key] = samples[key].combine(mutual_information_sample);
                }
            }
        }

        void add_fixed_mutual_information_samples(data_t &samples, uint32_t index) const {
            std::vector<uint32_t> interval1 = to_interval(x1, x2);
            std::vector<uint32_t> interval2 = to_interval(x3, x4);
            std::vector<uint32_t> interval3 = to_combined_interval(x1, x2, x3, x4);

            samples.emplace("mutual_information" + std::to_string(index), 
                            entropy(interval1, index) + entropy(interval2, index) - entropy(interval3, index));
        }

        void add_entropy_all_partition_sizes(data_t &samples, uint32_t index) const {
            for (uint32_t i = 0; i < system_size; i++) {
                Sample s = spatially_average ? spatially_averaged_entropy(system_size, i, spacing, index) : cum_entropy(i, index);
                samples.emplace("entropy" + std::to_string(index) + "_" + std::to_string(i), s);
            }
        }

        void add_avalanche_samples(data_t &samples, uint32_t index) {
            uint32_t total_avalanches = 0;
            for (uint32_t i = 0; i < num_av_bins; i++)
                total_avalanches += avalanche_size_samples[index][i];

            if (total_avalanches == 0) {
                for (uint32_t i = 0; i < num_av_bins; i++)
                    samples.emplace("avalanche" + std::to_string(index) + "_" + std::to_string(i), 0.0);
            } else {
                for (uint32_t i = 0; i < num_av_bins; i++)
                    samples.emplace("avalanche" + std::to_string(index) + "_" + std::to_string(i), double(avalanche_size_samples[index][i])/total_avalanches);
            }

            avalanche_size_samples[index] = std::vector<uint32_t>(num_av_bins, 0u);
        }

        virtual data_t take_samples() override {
            data_t samples;

            for (auto const &i : renyi_indices) {
                if (sample_entropy)
                    add_entropy_samples(samples, i);
                
                if (sample_all_partition_sizes)
                    add_entropy_all_partition_sizes(samples, i);

                if (sample_mutual_information)
                    add_mutual_information_samples(samples, i);

                if (sample_fixed_mutual_information)
                    add_fixed_mutual_information_samples(samples, i);

                if (sample_avalanche_size)
                    add_avalanche_samples(samples, i);
            }

            return samples;
        }

};

// TODO find a better place for this to live
static inline uint32_t mod(int a, int b) {
	int c = a % b;
	return (c < 0) ? c + b : c;
}
