#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumState.h>



class GroverProjectionSimulator : public EntropySimulator {
	private:
		std::binomial_distribution<uint32_t> dist;

		float mzr_prob;
		uint32_t nmax;
		double eps;

		std::string projection_type;


		bool offset;

	public:
		std::shared_ptr<UnitaryState> state;

		Eigen::MatrixXcd create_oracle(uint32_t z, const std::vector<uint32_t>& qubits) const;
		Eigen::MatrixXcd create_oracle(uint32_t z) const;

		void grover_projection(uint32_t z, const std::vector<uint32_t>& qubits);
		void grover_projection(uint32_t qubit);
		void random_grover_projection();

		GroverProjectionSimulator(Params &params);

		virtual void init_state(uint32_t num_threads) override;
		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits, index); }
		virtual void timesteps(uint32_t num_steps) override;

		CLONE(Simulator, GroverProjectionSimulator)
};