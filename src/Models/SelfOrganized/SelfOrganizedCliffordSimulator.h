#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumGraphState.h>

enum EvolutionType {
	RandomClifford,
	QuantumAutomaton,
	GraphOperations
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

		uint32_t gate_width;

		float threshold;
		float dx;

		bool initial_offset;

		uint32_t avalanche_size;

		
		uint32_t dist(int i, int j) const;
		float avg_dist() const;

		float max_component_size() const;

		void mzr(uint32_t q);
		void mzr_feedback();

		void qa_timestep(bool offset, bool gate_type);
		void qa_timesteps(uint32_t num_steps);
		void rc_timesteps(uint32_t num_steps);
		void graph_timesteps(uint32_t num_steps);

		// Self-organization strategies
		void random_measure();
		void cluster_threshold();
		void distance_threshold();

		void add_distance_distribution(data_t &samples) const;

	public:
		SelfOrganizedCliffordSimulator(Params &params);

		virtual void init_state(uint32_t) override;
		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits); }
		virtual void timesteps(uint32_t num_steps) override;

		virtual data_t take_samples() override;

		CLONE(Simulator, SelfOrganizedCliffordSimulator)
};