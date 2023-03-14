#ifndef SOC_RC_SIM_H
#define SOC_RC_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumGraphState.h"

#define DEFAULT_CLUSTER_THRESHOLD 0.5

class SelfOrganizedRandomCliffordSimulator : public EntropySimulator {
	private:
		std::unique_ptr<QuantumGraphState> state;
		float mzr_prob;
		float x;
		uint gate_width;
		uint soc_type;
		float cluster_threshold;

		bool initial_offset;

		Sample avalanche_size;

	public:
		SelfOrganizedRandomCliffordSimulator(Params &params);

		// Self-organization strategies
		void random_measure();
		void cluster_measure();
		void p_adjust();

		virtual void init_state();

		virtual float entropy(std::vector<uint> &qubits) const { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps);
		virtual std::unique_ptr<Simulator> clone(Params &params) { return std::unique_ptr<Simulator>(new SelfOrganizedRandomCliffordSimulator(params)); }
		virtual std::map<std::string, Sample> take_samples();
};

#endif