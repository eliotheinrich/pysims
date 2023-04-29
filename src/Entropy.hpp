#ifndef SIM_H
#define SIM_H

#include <iostream>
#include <DataFrame.hpp>
#include <Simulator.hpp>
#include <algorithm>
#include <numeric>
#include <math.h>

#define DEFAULT_SAMPLE_ENTROPY true

#define DEFAULT_SAMPLE_ALL_PARTITION_SIZES false

#define DEFAULT_SAMPLE_SURFACE_AVALANCHE false

#define DEFAULT_SAMPLE_MUTUAL_INFORMATION false
#define DEFAULT_NUM_BINS 100
#define DEFAULT_MIN_ETA 0.01
#define DEFAULT_MAX_ETA 1.0

#define DEFAULT_SAMPLE_FIXED_MUTUAL_INFORMATION false

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
        uint cum_entropy(uint i) const {
            std::vector<uint> sites(i+1);
            std::iota(sites.begin(), sites.end(), 0);

            return std::round(entropy(sites));
        }

        std::vector<uint> to_interval(uint x1, uint x2) const {
            assert(x1 < system_size);
            assert(x2 <= system_size);
            if (x2 == system_size) x2 = 0;
            std::vector<uint> interval;
            uint i = x1;
            while (true) {
                interval.push_back(i);
                i = (i + 1) % system_size;
                if (i == x2) {
                    //interval.push_back(i);
                    return interval;
                }
            }
        }

        std::vector<uint> to_combined_interval(int x1, int x2, int x3, int x4) const {
            std::vector<uint> interval1 = to_interval(x1, x2);
            std::sort(interval1.begin(), interval1.end());
            std::vector<uint> interval2 = to_interval(x3, x4);
            std::sort(interval2.begin(), interval2.end());
            std::vector<uint> combined_interval;


            std::set_union(interval1.begin(), interval1.end(), 
                           interval2.begin(), interval2.end(), 
                           std::back_inserter(combined_interval));

            return combined_interval;
        }

        float get_x(int x1, int x2) const {
            return std::sin(std::abs(x1 - x2)*M_PI/system_size);
        }

        float get_eta(int x1, int x2, int x3, int x4) const {
            float x12 = get_x(x1, x2);
            float x34 = get_x(x3, x4);
            float x13 = get_x(x1, x3);
            float x24 = get_x(x2, x4);
            return x12*x34/(x13*x24);
        }

		std::vector<float> compute_entropy_table() const {
			std::vector<float> table;

			for (uint x1 = 0; x1 < system_size; x1++) {
				uint x2 = (x1 + partition_size) % system_size;

				std::vector<uint> sites = to_interval(x1, x2);
				table.push_back(entropy(sites));
			}

			return table;
		}

    public:
        uint system_size;
        uint partition_size;
        uint spacing;

        Entropy() {}

        Entropy(Params &params) {
            system_size = params.get<int>("system_size");
            partition_size = params.get<int>("partition_size", 0); // A partition_size value of 0 will result in measuring every possible partition size
            spacing = params.get<int>("spacing", DEFAULT_SPACING);
        }

        // This is the primary virtual function which must be overloaded by inheriting classes
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


        std::vector<Sample> entropy_surface() const {
            std::vector<Sample> surface;

            std::vector<uint> sites;
            for (uint i = 0; i < system_size; i++) {
                sites.push_back(i);
                for (uint j = 0; j < system_size; j++) {
                    std::vector<uint> _sites(sites);
                    std::transform(_sites.begin(), _sites.end(), _sites.begin(), [system_size=system_size, j=j](uint q) { return (q + j) % system_size; } );
                    surface.push_back(entropy(_sites));
                }
            }

            return surface;
        }

};

class EntropySimulator : public Simulator, public Entropy {
    protected:
        bool sample_entropy;

        bool sample_all_partition_sizes;

        bool sample_surface_avalanche;
        std::vector<uint> entropy_surface_vec;

        bool sample_mutual_information;
        uint num_bins;
        float min_eta;
        float max_eta;
        float bin_width;

        bool sample_fixed_mutual_information;
        uint x1;
        uint x2;
        uint x3;
        uint x4;

        std::vector<float> bins() const {
            float h = bin_width;
            std::vector<float> bin;
            for (uint i = 0; i < num_bins; i++) bin.push_back(i*h + min_eta);

            return bin;
        }

        uint get_bin_idx(float eta) const {
            for (uint i = 0; i < num_bins; i++) {
                if (eta < i*bin_width + min_eta) {
                    return i;
                }
            }
            std::cout << "Could not find valid index for eta.\n";
            assert(false);
            return -1;
        }


    public:
        EntropySimulator(Params &params) : Simulator(params), Entropy(params), 
                                           entropy_surface_vec(std::vector<uint>(system_size, 0)) {
                                            
            sample_entropy = params.get<int>("sample_entropy", DEFAULT_SAMPLE_ENTROPY);
            sample_all_partition_sizes = params.get<int>("sample_all_partition_sizes", DEFAULT_SAMPLE_ALL_PARTITION_SIZES);
            sample_surface_avalanche = params.get<int>("sample_surface_avalanche", DEFAULT_SAMPLE_SURFACE_AVALANCHE);
            sample_mutual_information = params.get<int>("sample_mutual_information", DEFAULT_SAMPLE_MUTUAL_INFORMATION);
            sample_fixed_mutual_information = params.get<int>("sample_fixed_mutual_information", DEFAULT_SAMPLE_FIXED_MUTUAL_INFORMATION);

            if (sample_mutual_information) {
                assert(partition_size > 0);
                num_bins = params.get<int>("num_bins", DEFAULT_NUM_BINS);
                min_eta = params.get<float>("min_eta", DEFAULT_MIN_ETA);
                max_eta = params.get<float>("max_eta", DEFAULT_MAX_ETA);

                bin_width = (max_eta - min_eta)/num_bins;
            }

            if (sample_fixed_mutual_information) {
                x1 = params.get<int>("x1");
                x2 = params.get<int>("x2");
                x3 = params.get<int>("x3");
                x4 = params.get<int>("x4");
            }
        }

        void add_surface_avalanche_samples(data_t &samples) {
            std::vector<uint> new_surface(system_size, 0);
            for (uint i = 0; i < system_size; i++) new_surface[i] = cum_entropy(i);

            uint avalanche_size = 0;
            for (uint i = 0; i < system_size; i++)
                if (new_surface[i] != entropy_surface_vec[i]) avalanche_size++;

            entropy_surface_vec = new_surface;
            samples.emplace("avalanche_size", avalanche_size);
        }

        void add_mutual_information_samples(data_t &samples) const {
            std::vector<float> entropy_table = compute_entropy_table();
            for (uint x1 = 0; x1 < system_size; x1++) {
                LOG("x1 = " << x1 << std::endl);
                for (uint x3 = 0; x3 < system_size; x3++) {
                    uint x2 = (x1 + partition_size) % system_size;
                    uint x4 = (x3 + partition_size) % system_size;

                    float eta = get_eta(x1, x2, x3, x4);
                    if (!(eta > min_eta) || !(eta < max_eta)) continue;

                    std::vector<uint> combined_sites = to_combined_interval(x1, x2, x3, x4);

                    float entropy1 = entropy_table[x1];
                    float entropy2 = entropy_table[x3];
                    float entropy3 = entropy(combined_sites);
                    
                    float mutual_information_sample = entropy1 + entropy2 - entropy3;

                    std::string key = "I_" + std::to_string(get_bin_idx(eta));
                    if (samples.count(key))
                        samples[key] = mutual_information_sample;
                    else
                        samples[key] = samples[key].combine(mutual_information_sample);
                }
            }
        }

        void add_fixed_mutual_information_samples(data_t &samples) const {
            std::vector<uint> interval1 = to_interval(x1, x2);
            std::vector<uint> interval2 = to_interval(x3, x4);
            std::vector<uint> interval3 = to_combined_interval(x1, x2, x3, x4);

            samples.emplace("mutual_information", entropy(interval1) + entropy(interval2) - entropy(interval3));
        }

        void add_entropy_all_partition_sizes(data_t &samples) const {
            for (uint i = 0; i < system_size; i++)
                samples.emplace("entropy_" + std::to_string(i), spatially_averaged_entropy(system_size, i, spacing));
        }

        virtual data_t take_samples() override {
            data_t samples;

            if (sample_entropy)
                samples.emplace("entropy", spatially_averaged_entropy());
            
            if (sample_all_partition_sizes)
                add_entropy_all_partition_sizes(samples);

            if (sample_surface_avalanche)
                add_surface_avalanche_samples(samples);

            if (sample_mutual_information)
                add_mutual_information_samples(samples);

            if (sample_fixed_mutual_information)
                add_fixed_mutual_information_samples(samples);

            return samples;
        }

};

// TODO find a better place for this to live
static inline const uint mod(int a, int b) {
	int c = a % b;
	return (c < 0) ? c + b : c;
}


#endif