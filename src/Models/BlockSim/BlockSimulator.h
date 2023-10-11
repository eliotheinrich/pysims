#pragma once

#include "Simulator.hpp"
#include <DataFrame.hpp>
#include <random>

class BlockSimulator : public Simulator {
    private:
        uint32_t system_size;
        float pu;
        float pm;
        std::vector<uint32_t> surface;

        bool random_sites;

        bool precut;

        bool start_sampling;
        std::vector<uint32_t> avalanche_sizes;

        uint32_t feedback_mode;
        std::vector<uint32_t> feedback_strategy;

        uint32_t depositing_type;

        bool sample_avalanche_sizes;

        uint32_t get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const;

        void avalanche(uint32_t i);
        bool can_deposit(uint32_t i) const;
        void deposit(uint32_t i);
        void record_size(uint32_t s) {
            if (start_sampling) 
                avalanche_sizes[s]++;
        }

    public:
        BlockSimulator(Params &params);

        virtual void init_state(uint32_t) override {
            surface = std::vector<uint32_t>(system_size, 0u);
        }


        virtual void timesteps(uint32_t num_steps) override;
        virtual void equilibration_timesteps(uint32_t num_steps) override {
            start_sampling = false;
            timesteps(num_steps);
            start_sampling = true;
        }

        std::string to_string() const;

        void add_avalanche_samples(data_t &samples);
        virtual data_t take_samples() override;

        CLONE(Simulator, BlockSimulator)
};