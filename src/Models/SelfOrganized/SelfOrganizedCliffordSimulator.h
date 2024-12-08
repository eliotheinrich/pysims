#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

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

class SelfOrganizedCliffordSimulator : public Simulator {
	private:
		uint32_t system_size;

		FeedbackType feedback_type;
		EvolutionType evolution_type;

		std::shared_ptr<QuantumGraphState> state;
		double mzr_prob;
		double x;

		uint32_t gate_width;

		double threshold;
		double dx;

		bool initial_offset;

		uint32_t avalanche_size;

		EntropySampler sampler;

		
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

		void add_distance_distribution(dataframe::data_t &samples) const;

	public:
		SelfOrganizedCliffordSimulator(dataframe::Params &params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::data_t take_samples() override;
};
