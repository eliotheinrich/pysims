#ifndef DEBUG_SIM_H
#define DEBUG_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

class DebugSimulator : public Entropy, public Config {
	private:
		std::unique_ptr<QuantumCHPState> state;

		uint sample_idx;

	public:
		DebugSimulator(Params &params) : Entropy(params), Config(params), sample_idx(0) {
			state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(system_size)); 
			for (uint i = 0; i < system_size; i++) state->h_gate(i);
		}

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }

		std::map<std::string, std::vector<Sample>> take_vector_samples() {
			std::map<std::string, std::vector<Sample>> samples;
			samples.emplace("surface_" + std::to_string(sample_idx), entropy_surface());
			sample_idx++;

			return samples;
		}

		DataSlide compute() {
			DataSlide slide;

			return slide;
		}

		virtual std::unique_ptr<Config> clone() { return std::unique_ptr<Config>(new DebugSimulator(params)); }
};

#endif