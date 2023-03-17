#ifndef CLUSTER_CLIFFORD_SIM_H
#define CLUSTER_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumGraphState.h"

#define DEFAULT_CLUSTER_THRESHOLD 0.5
#define DEFAULT_CIRCUIT 0

class ClusterCliffordSimulator : public EntropySimulator {
	private:
		std::unique_ptr<QuantumGraphState> state;
		float mzr_prob;
		float x;
		uint gate_width;
		uint soc_type;
		float cluster_threshold;

		bool initial_offset;

		int circuit_type;

		Sample avalanche_size;

		void mzr(uint q);
		void mzr_feedback();

		void qa_timestep(bool offset, bool gate_type);
		void qa_timesteps(uint num_steps);
		void rc_timesteps(uint num_steps);

		// Self-organization strategies
		void random_measure();
		void cluster_measure();
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