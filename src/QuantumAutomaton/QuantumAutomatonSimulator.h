#ifndef QASIM_H
#define QASIM_H
#include <DataFrame.hpp>
#include "Simulator.hpp"
#include "CliffordState.hpp"

class QuantumAutomatonSimulator : public Simulator, public Entropy {
	private:
		CliffordState *state;
		float mzr_prob;

	public:
		QuantumAutomatonSimulator() {};
		QuantumAutomatonSimulator(uint system_size, float mzr_prob, CliffordType simulator_type);
		~QuantumAutomatonSimulator();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		
		void timestep(bool offset, bool gate_type);
		virtual void timesteps(uint num_steps);
};

#endif