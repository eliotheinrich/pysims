#ifndef QASIM_H
#define QASIM_H
#include <DataFrame.hpp>
#include "Simulator.hpp"
#include "CliffordState.hpp"

class QuantumAutomatonSimulator : public EntropySimulator {
	private:
		CliffordState *state;
		float mzr_prob;

	public:
		QuantumAutomatonSimulator() : {};
		QuantumAutomatonSimulator(Params &params);
		~QuantumAutomatonSimulator();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		
		void timestep(bool offset, bool gate_type);
		virtual void timesteps(uint num_steps);
};

#endif