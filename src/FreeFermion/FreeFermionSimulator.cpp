#include "FreeFermionSimulator.h"
#include <iostream>

template <typename T>
void print_vector(std::vector<T>& v) {
    std::cout << "[";
    for (auto t : v) {
        std::cout << t << ", ";
    }
    std::cout << "]\n";
}


FreeFermionSimulator::FreeFermionSimulator() {}

FreeFermionSimulator::FreeFermionSimulator(uint system_size, float p1, float p2, float beta, float filling_fraction) : system_size(system_size), p1(p1), p2(p2), beta(beta) {
    this->num_particles = filling_fraction * system_size;
    this->propagator = Eigen::MatrixXcd::Identity(system_size, system_size);
}

int FreeFermionSimulator::get_num_particles() {
    return num_particles;
}

float FreeFermionSimulator::kappa() {
    float r = float((rng)())/float(RAND_MAX);
    if (r < p1) {
        return 1.;
    } else {
        return -1.;
    }
}

float FreeFermionSimulator::lambda() {
    float r = float((rng)())/float(RAND_MAX);
    if (r < p2) {
        return 1.;
    } else {
        return 0.;
    }
}

void FreeFermionSimulator::real_timestep() {
    Eigen::MatrixXcd H = Eigen::MatrixXcd::Zero(system_size, system_size);

    for (int i = 0; i < system_size; i++) {
        float k = kappa();
        int j = (i + 1) % system_size;
        H(i, j) = k;
        H(j, i) = k;
    }

    Eigen::MatrixXcd U = (-2.*std::complex<double>(0., 1.)*H).array().exp();

    propagator = U * propagator;
}

void FreeFermionSimulator::imag_timestep() {
    Eigen::VectorXf H(system_size);

    for (int i = 0; i < system_size; i++) {
        propagator(i) *= std::exp(-2.*beta*lambda());
    }
}

void FreeFermionSimulator::timesteps(uint num_steps) {
    for (int i = 0; i < num_steps; i++) {
        real_timestep();
        imag_timestep();
    }
}

Eigen::MatrixXcd FreeFermionSimulator::correlation_function() const {
    Eigen::MatrixXcd W = propagator(Eigen::all, Eigen::seq(0, num_particles));

    return (W * W.adjoint()).transpose();
}

float FreeFermionSimulator::entropy(std::vector<uint> &sites) const {
    uint subsystem_size = sites.size();
    Eigen::MatrixXcd C = correlation_function();
    
    Eigen::MatrixXcd C1 = C(sites, sites);
    Eigen::MatrixXcd C2 = Eigen::MatrixXcd::Identity(subsystem_size, subsystem_size) - C1;

    float s = -(C1.array() * C1.array().log() + C2.array() * C2.array().log()).matrix().trace().real();
    return s;
}