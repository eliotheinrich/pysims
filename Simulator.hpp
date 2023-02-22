#include <DataFrame.hpp>

class Simulator {
    public:
        virtual void timesteps(unsigned int num_steps)=0;
};

class CliffordSimulator: public Simulator, public Entropy {
    public:
        void h_gate(uint a);
        void s_gate(uint a);
        void cz_gate(uint a, uint b);

};

class Entropy {
    public:
        virtual float entropy(std::vector<uint> &sites) const=0;

        Sample spatially_averaged_entropy(uint system_size, uint partition_size, uint spacing) {
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

            float stdev = std::sqrt(s2 - s*s);

            return Sample(s, stdev, num_partitions);        
        }
};