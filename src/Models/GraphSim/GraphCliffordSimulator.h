#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

class GraphCliffordSimulator : public Simulator {
	private:
		uint32_t system_size;

		std::string evolution_type;

		std::shared_ptr<QuantumGraphState> state;
		float mzr_prob;

		uint32_t gate_width;
		bool initial_offset;

		uint32_t m;
		float a;

		std::minstd_rand rng;

		EntropySampler sampler;

		void mzr(uint32_t q);

		void unitary_timesteps(uint32_t num_steps);
		void qa_timesteps(uint32_t num_steps);
		void rc_timesteps(uint32_t num_steps);
		void generate_random_graph();

		uint32_t dist(int i, int j) const;
		void add_distance_distribution(dataframe::SampleMap &samples) const;
		void add_degree_distribution(dataframe::SampleMap &samples) const;
		void add_avg_max_dist(dataframe::SampleMap &samples) const;


	public:
		GraphCliffordSimulator(dataframe::ExperimentParams &params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;
		virtual dataframe::SampleMap take_samples() override;
};
