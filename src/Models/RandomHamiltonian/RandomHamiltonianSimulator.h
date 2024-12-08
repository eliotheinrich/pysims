#pragma once

#include <Simulator.hpp>
#include <QuantumState.h>
#include <Samplers.h>

class RandomHamiltonianSimulator : public Simulator {
	private:
		uint32_t system_size;

    double dt;
    double mu;
    double sigma;

		EntropySampler entropy_sampler;
		QuantumStateSampler prob_sampler;

    Eigen::MatrixXcd hamiltonian;
    Eigen::MatrixXcd evolution_operator;

	public:
		std::shared_ptr<Statevector> state;

		RandomHamiltonianSimulator(dataframe::Params &params, uint32_t num_threads);

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::data_t take_samples() override;
};
