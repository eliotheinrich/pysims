#ifndef FF_SIM_H
#define FF_SIM_H


#include <Eigen/Core>
#include <vector>
#include <random>
#include <DataFrame.hpp>
#include "Simulator.hpp"


class FreeFermionSimulator : public EntropyConfig {
    private:
        Eigen::MatrixXcd propagator;

        int system_size;
        int num_particles;
        float beta;
        float p1;
        float p2;
        std::minstd_rand rng;


    public:
        FreeFermionSimulator();
        FreeFermionSimulator(uint system_size, float p1, float p2, float beta, float filling_fraction);
        ~FreeFermionSimulator();
        
        int get_num_particles();

        float kappa();
        float lambda();

        void real_timestep();
        void imag_timestep();
        void timesteps(uint num_timesteps);

        Eigen::MatrixXcd correlation_function() const;

        virtual float entropy(std::vector<uint> &sites) const;
};

#endif