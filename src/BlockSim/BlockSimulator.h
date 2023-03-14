#ifndef BLOCKSIM_H
#define BLOCKSIM_H

#include "Simulator.hpp"
#include <DataFrame.hpp>
#include <random>

#define DEFAULT_RANDOM_SITE_SELECTION false
#define DEFAULT_AVALANCHE_TYPE 0

class BlockSimulator : public Simulator {
    private:
        uint system_size;
        float prob;
        std::vector<uint> *surface;

        bool random_sites;
        uint avalanche_type;

        Sample avalanche_size;

        void unitary_timestep();
        void projective_timestep();
        int slope(uint i) const;
        void uphill_avalanche(uint i);
        void waterline_avalanche(uint i);

        void unitary_stack(uint i);

    public:
        BlockSimulator(Params &params);
        ~BlockSimulator() { delete surface; }

        virtual void init_state();

        virtual void timesteps(uint num_steps);
        std::string to_string() const;

        virtual std::map<std::string, Sample> take_samples();

        CLONE(Simulator, BlockSimulator)
};

#endif