#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

class BlockSimulator : public dataframe::Simulator {
  private:
    uint32_t system_size;
    double pu;
    double pm;
    std::vector<int> surface;

    bool random_sites;

    bool precut;

    bool start_sampling;

    uint32_t feedback_mode;
    std::vector<uint32_t> feedback_strategy;

    uint32_t depositing_type;
    uint32_t avalanche_type;
    double delta;
    double normalization;

    InterfaceSampler sampler;

    uint32_t get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const;

    double powerlaw(double d) const;
    bool can_desorb(uint32_t i) const;
    void avalanche(uint32_t i);
    bool can_deposit(uint32_t i) const;
    void deposit(uint32_t i);

  public:
    BlockSimulator(dataframe::Params &params, uint32_t);

    virtual void timesteps(uint32_t num_steps) override;
    virtual void equilibration_timesteps(uint32_t num_steps) override {
      start_sampling = false;
      timesteps(num_steps);
      start_sampling = true;
    }

    std::string to_string() const;

    virtual dataframe::data_t take_samples() override;
};
