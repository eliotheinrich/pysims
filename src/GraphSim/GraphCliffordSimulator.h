#ifndef GS_CLIFFORD_SIM_H
#define GS_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumGraphState.h"

enum GraphSimCircuitType {
	GraphSimRandomClifford,
	GraphSimQuantumAutomaton,
	GraphSimUnitary,
};

class GraphCliffordSimulator : public EntropySimulator {
	private:
		GraphSimCircuitType circuit_type;

		std::shared_ptr<QuantumGraphState> state;
		float mzr_prob;

		uint gate_width;
		bool initial_offset;

		void mzr(uint q);

		void unitary_timesteps(uint num_steps);
		void qa_timesteps(uint num_steps);
		void rc_timesteps(uint num_steps);

		uint dist(int i, int j) const;
		void add_distance_distribution(std::map<std::string, Sample> &samples) const;
		void add_degree_distribution(std::map<std::string, Sample> &samples) const;
		void add_avg_max_dist(std::map<std::string, Sample> &samples) const;


	public:
		GraphCliffordSimulator(Params &params);

		virtual void init_state() override;

		virtual float entropy(std::vector<uint> &qubits) const override { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps) override;
		virtual std::map<std::string, Sample> take_samples() override;

		CLONE(Simulator, GraphCliffordSimulator)
};

#endif