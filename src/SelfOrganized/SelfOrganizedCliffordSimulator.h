#ifndef SOC_CLIFFORD_SIM_H
#define SOC_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumGraphState.h"

enum EvolutionType {
	RandomClifford,
	QuantumAutomaton
};

enum FeedbackType {
	NoFeedback,
	ClusterThreshold,
	DistanceThreshold
};

class SelfOrganizedCliffordSimulator : public EntropySimulator {
	private:
		FeedbackType feedback_type;
		EvolutionType evolution_type;

		std::unique_ptr<QuantumGraphState> state;
		float mzr_prob;
		float x;

		uint gate_width;

		float threshold;
		float dx;

		bool initial_offset;

		uint avalanche_size;

		
		uint dist(int i, int j) const;
		float avg_dist() const;

		float max_component_size() const;

		void mzr(uint q);
		void mzr_feedback();

		void qa_timestep(bool offset, bool gate_type);
		void qa_timesteps(uint num_steps);
		void rc_timesteps(uint num_steps);

		// Self-organization strategies
		void random_measure();
		void cluster_threshold();
		void distance_threshold();


	public:
		SelfOrganizedCliffordSimulator(Params &params);

		virtual void init_state() override ;

		virtual float entropy(std::vector<uint> &qubits) const override { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps) override;


		virtual data_t take_samples() override;

		CLONE(Simulator, SelfOrganizedCliffordSimulator)
};

#endif