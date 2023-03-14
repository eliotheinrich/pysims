#ifndef QASIM_H
#define QASIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "CliffordState.hpp"

#define DEFAULT_CLIFFORD_TYPE "chp"
#define DEFAULT_SEED -1

class QuantumAutomatonSimulator : public EntropySimulator {
	private:
		std::unique_ptr<CliffordState> state;
		CliffordType clifford_type;
		float mzr_prob;
		int random_seed;

	public:
		QuantumAutomatonSimulator(Params &params);

		virtual void init_state();
		
		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		
		void timestep(bool offset, bool gate_type);
		virtual void timesteps(uint num_steps);

		CLONE(Simulator, QuantumAutomatonSimulator)
};

#endif