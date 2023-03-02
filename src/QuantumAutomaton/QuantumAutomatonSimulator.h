#ifndef QASIM_H
#define QASIM_H
#include <DataFrame.hpp>
#include "Simulator.hpp"
#include "CliffordState.hpp"

#define DEFAULT_CLIFFORD_TYPE "chp"

class QuantumAutomatonSimulator : public EntropySimulator {
	private:
		CliffordState *state;
		CliffordType clifford_type;
		float mzr_prob;

	public:
		QuantumAutomatonSimulator(Params &params);
		~QuantumAutomatonSimulator();

		virtual void init_state();
		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		
		void timestep(bool offset, bool gate_type);
		virtual void timesteps(uint num_steps);
		virtual Simulator* clone(Params &params) { return new QuantumAutomatonSimulator(params); }
};

#endif