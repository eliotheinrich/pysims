#ifndef RC_SIM_H
#define RC_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "CliffordState.hpp"

#define DEFAULT_CLIFFORD_TYPE "chp"
#define DEFAULT_SEED -1

class RandomCliffordSimulator : public EntropySimulator {
	private:
		std::unique_ptr<CliffordState> state;

		CliffordType clifford_type;
		float mzr_prob;
		uint gate_width;
		
		int random_seed;
		bool initial_offset;

	public:
		RandomCliffordSimulator(Params &params);

		virtual void init_state();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps);
		virtual std::unique_ptr<Simulator> clone(Params &params) { return std::unique_ptr<Simulator>(new RandomCliffordSimulator(params)); }
};

#endif