#ifndef SOC_CLIFFORD_SIM_H
#define SOC_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

#define DEFAULT_RANDOM_SITES true

class SelfOrganizedCliffordSimulator : public EntropySimulator {
	private:
		std::unique_ptr<QuantumCHPState> state;

		float unitary_prob;
		float mzr_prob;

		bool random_sites;

		int cum_entropy(uint i) const;

	public:
		SelfOrganizedCliffordSimulator(Params &params);

		virtual void init_state() { 
			state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(system_size)); 
		}

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }

		void mzr(uint i);
		void unitary(uint i);
		void timestep();
		virtual void timesteps(uint num_steps);

		CLONE(Simulator, SelfOrganizedCliffordSimulator)
};

#endif