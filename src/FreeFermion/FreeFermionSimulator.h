#ifndef FF_SIM_H
#define FF_SIM_H


#include <Eigen/Core>
#include <vector>
#include <random>
#include <DataFrame.hpp>
#include "Simulator.hpp"

class FreeFermionSimulator : public EntropySimulator {
    private:
        Eigen::MatrixXcd propagator;

        int system_size;
        int num_particles;
        float beta;
        float p1;
        float p2;

    public:
        FreeFermionSimulator(Params &params);

        virtual void init_state() override;

        int get_num_particles();

        float kappa();
        float lambda();

        void real_timestep();
        void imag_timestep();
        virtual void timesteps(uint num_timesteps) override;

        Eigen::MatrixXcd correlation_function() const;

        virtual float entropy(std::vector<uint> &sites) const override;

        CLONE(Simulator, FreeFermionSimulator)
};

#endif