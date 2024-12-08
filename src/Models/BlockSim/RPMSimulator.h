#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

class RPMSimulator : public Simulator {
  private:
    uint32_t system_size;
    uint32_t num_sites;
    float pu;
    float pm;
    bool pbc;
    uint32_t initial_state;

    std::vector<int> surface;

    bool start_sampling;

    InterfaceSampler sampler;

    void peel(uint32_t i, bool right);
    void raise(uint32_t i);
    int slope(uint32_t i) const;

  public:
    RPMSimulator(dataframe::Params &params, uint32_t);

    virtual void timesteps(uint32_t num_steps) override;
    virtual void equilibration_timesteps(uint32_t num_steps) override {
      start_sampling = false;
      timesteps(num_steps);
      start_sampling = true;
    }

    std::string to_string() const;

    virtual dataframe::data_t take_samples() override;
};
