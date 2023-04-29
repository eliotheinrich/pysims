#ifndef SANDPILE_CLIFFORD_SIM_H
#define SANDPILE_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

enum BoundaryCondition {
	Periodic,
	Open1,
	Open2
};

class SandpileCliffordSimulator : public EntropySimulator {
	private:
		std::unique_ptr<QuantumCHPState> state;

		float unitary_prob;
		float mzr_prob;

		uint system_size;

		bool random_sites;
		BoundaryCondition boundary_condition;
		uint feedback_mode;
		
		bool direction;
		bool performed_unitary;
		bool performed_mzr;

		std::vector<uint> feedback_strategy;

		bool start_sampling;
		bool sample_transition_matrix;
		std::vector<std::vector<uint>> transition_matrix_unitary;
		std::vector<std::vector<uint>> transition_matrix_mzr;

		void feedback(uint q);

		void left_boundary();
		void right_boundary();

		void mzr(uint i);
		void unitary(uint i);
		
		void timestep();

		uint sp_cum_entropy_left(uint i) const;
		uint sp_cum_entropy_right(uint i) const;
		uint sp_cum_entropy(uint i) const;

		uint get_shape(uint s0, uint s1, uint s2) const;

	public:
		SandpileCliffordSimulator(Params &params);

		virtual void init_state() override { 
			state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(system_size)); 
		}

		virtual float entropy(std::vector<uint> &qubits) const override { return state->entropy(qubits); }

		virtual void timesteps(uint num_steps) override;
		virtual void equilibration_timesteps(uint num_steps) override {
			start_sampling = false;
			timesteps(num_steps);
			start_sampling = true;
		}

		void add_transition_matrix_samples(data_t &samples);
		virtual data_t take_samples() override;

		CLONE(Simulator, SandpileCliffordSimulator)
};

#endif