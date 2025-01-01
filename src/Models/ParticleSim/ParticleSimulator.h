#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

class ParticleSimulator : public Simulator {
  private:
    uint32_t system_size;
    double pu;
    double pm;
    std::vector<int> state;

    InterfaceSampler sampler;

    uint32_t get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const;

  public:
    ParticleSimulator(dataframe::ExperimentParams &params, uint32_t);

    virtual void timesteps(uint32_t num_steps) override;

    std::string to_string() const;

    virtual dataframe::data_t take_samples() override;
};
