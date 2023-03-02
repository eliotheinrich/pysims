#ifndef CLUSTER_RC_SIM_H
#define CLUSTER_RC_SIM_H
#include <DataFrame.hpp>
#include "Simulator.hpp"
#include "QuantumGraphState.h"

class ClusterRandomCliffordSimulator : public EntropySimulator {
	private:
		QuantumGraphState *state;
		float mzr_prob;
		uint gate_width;

		bool initial_offset;

	public:
		ClusterRandomCliffordSimulator(Params &params);
		~ClusterRandomCliffordSimulator();

		virtual void init_state();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps);
		virtual Simulator* clone(Params &params) { return new ClusterRandomCliffordSimulator(params); }
};

#endif