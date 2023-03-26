#ifndef GS_CLIFFORD_SIM_H
#define GS_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumGraphState.h"

enum GSCircuitType {
	GSRandomClifford,
	GSQuantumAutomaton
};

static GSCircuitType parse_gscircuit_type(std::string s) {
	if (s == "random_clifford") return GSCircuitType::GSRandomClifford;
	else if (s == "quantum_automaton") return GSCircuitType::GSQuantumAutomaton;
	else {
		std::cout << "Invalid circuit type: " << s << std::endl;
		assert(false);
	}
}

#define DEFAULT_CIRCUIT_TYPE "random_clifford"

class GraphCliffordSimulator : public EntropySimulator {
	private:
		GSCircuitType circuit_type;

		std::unique_ptr<QuantumGraphState> state;
		float mzr_prob;

		uint gate_width;

		bool initial_offset;

		void mzr(uint q);

		void qa_timestep(bool offset, bool gate_type);
		void qa_timesteps(uint num_steps);
		void rc_timesteps(uint num_steps);

		float dist(int i, int j) const;
		std::pair<float, float> avg_max_dist() const;


	public:
		GraphCliffordSimulator(Params &params);

		virtual void init_state();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps);
		virtual std::map<std::string, Sample> take_samples();

		CLONE(Simulator, GraphCliffordSimulator)
};

#endif