#pragma once

#include <Simulator.hpp>
#include <InterfaceSampler.hpp>

class BlockSimulator : public Simulator {
  private:
    uint32_t system_size;
    float pu;
    float pm;
    std::vector<int> surface;

    bool random_sites;

    bool precut;

    bool start_sampling;

    uint32_t feedback_mode;
    std::vector<uint32_t> feedback_strategy;

    uint32_t depositing_type;

    InterfaceSampler sampler;

    uint32_t get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const;

    void avalanche(uint32_t i);
    bool can_deposit(uint32_t i) const;
    void deposit(uint32_t i);

  public:
    BlockSimulator(Params &params, uint32_t);

    virtual void timesteps(uint32_t num_steps) override;
    virtual void equilibration_timesteps(uint32_t num_steps) override {
      start_sampling = false;
      timesteps(num_steps);
      start_sampling = true;
    }

    std::string to_string() const;

    virtual data_t take_samples() override;
};
