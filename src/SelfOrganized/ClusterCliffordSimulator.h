#ifndef CLUSTER_CLIFFORD_SIM_H
#define CLUSTER_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumGraphState.h"

enum CircuitType {
	RandomClifford,
	QuantumAutomaton
};

enum FeedbackType {
	NoFeedback,
	ClusterMzr,
	PAdjust
};

#define DEFAULT_CLUSTER_THRESHOLD 0.5
#define DEFAULT_CIRCUIT_TYPE "random_clifford"
#define DEFAULT_DX 0.05

class ClusterCliffordSimulator : public EntropySimulator {
	private:
		FeedbackType feedback_type;
		CircuitType circuit_type;

		std::unique_ptr<QuantumGraphState> state;
		float mzr_prob;
		float x;

		uint gate_width;
		float cluster_threshold;
		float dx;

		bool initial_offset;

		uint avalanche_size;

		void mzr(uint q);
		void mzr_feedback();

		void qa_timestep(bool offset, bool gate_type);
		void qa_timesteps(uint num_steps);
		void rc_timesteps(uint num_steps);

		// Self-organization strategies
		void random_measure();
		void cluster_mzr();
		void p_adjust();


	public:
		ClusterCliffordSimulator(Params &params);

		virtual void init_state();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps);
		virtual std::map<std::string, Sample> take_samples();

		CLONE(Simulator, ClusterCliffordSimulator)
};

#endif