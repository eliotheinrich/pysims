#ifndef RC_SIM_H
#define RC_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "CliffordState.hpp"

#define DEFAULT_CLIFFORD_TYPE "chp"

class RandomCliffordSimulator : public EntropySimulator {
	private:
		std::unique_ptr<CliffordState> state;

		CliffordType clifford_type;
		float mzr_prob;
		uint gate_width;
		
		bool initial_offset;

	public:
		RandomCliffordSimulator(Params &params);

		virtual void init_state();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps);

		CLONE(Simulator, RandomCliffordSimulator)
};

#endif