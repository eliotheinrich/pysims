#ifndef BELL_CLIFFORD_SIM_H
#define BELL_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

class BellSandpileSimulator : public EntropySimulator {
	private:
		std::unique_ptr<QuantumCHPState> state;

		float pu;
		float pm;

		bool random_sites;

		uint gate_type;

		uint feedback_mode;
		std::vector<uint> feedback_strategy;

		void feedback(uint q);
		void unitary(uint i);
		void mzr(uint i);
		void timestep();

		uint get_shape(uint s0, uint s1, uint s2) const;

	public:
		BellSandpileSimulator(Params &params);

		virtual void init_state() override { 
			state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(2*system_size)); 
			for (uint i = 0; i < 2*system_size; i++)
				state->h_gate(i);
		}

		virtual float entropy(const std::vector<uint> &qubits) const override { return state->entropy(qubits); }

		virtual void timesteps(uint num_steps) override;

		virtual data_t take_samples() override;

		CLONE(Simulator, BellSandpileSimulator)
};

#endif