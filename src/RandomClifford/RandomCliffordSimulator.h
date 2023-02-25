#ifndef RC_SIM_H
#define RC_SIM_H
#include <DataFrame.hpp>
#include "Simulator.hpp"
#include "CliffordState.hpp"

class RandomCliffordSimulator : public Simulator, public Entropy {
	private:
		CliffordState *state;
		float mzr_prob;
		uint gate_width;
		bool initial_offset;

	public:
		RandomCliffordSimulator() {}
		RandomCliffordSimulator(uint system_size, float mzr_prob, uint gate_width, CliffordType simulator_type);
		~RandomCliffordSimulator();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }

		virtual void timesteps(uint num_steps);
};

#endif