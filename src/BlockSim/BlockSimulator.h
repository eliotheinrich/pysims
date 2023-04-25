#ifndef BLOCKSIM_H
#define BLOCKSIM_H

#include "Simulator.hpp"
#include <DataFrame.hpp>
#include <random>

class BlockSimulator : public Simulator {
    private:
        uint system_size;
        float pu;
        float pm;
        std::vector<uint> surface;

        bool random_sites;

        bool precut;

        bool start_sampling;
        std::vector<uint> avalanche_sizes;

        uint feedback_mode;
        std::vector<uint> feedback_strategy;

        bool sample_avalanche_sizes;

        uint get_shape(uint i) const;

        void avalanche(uint i);
        void stack(uint i);
        void record_size(uint s) {
            if (start_sampling) avalanche_sizes[s]++;
        }

    public:
        BlockSimulator(Params &params);

        virtual void init_state() override;

        virtual void timesteps(uint num_steps) override;
        virtual void equilibration_timesteps(uint num_steps) override {
            start_sampling = false;
            timesteps(num_steps);
            start_sampling = true;
        }

        std::string to_string() const;

        void add_avalanche_samples(std::map<std::string, Sample> &samples);
        virtual std::map<std::string, Sample> take_samples() override;

        CLONE(Simulator, BlockSimulator)
};

#endif