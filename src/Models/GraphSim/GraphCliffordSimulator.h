#pragma once

#include <Simulator.hpp>
#include <QuantumGraphState.h>

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
		void add_distance_distribution(data_t &samples) const;
		void add_degree_distribution(data_t &samples) const;
		void add_avg_max_dist(data_t &samples) const;


	public:
		GraphCliffordSimulator(Params &params);

		virtual void init_state(uint32_t) override;

		virtual void timesteps(uint32_t num_steps) override;
		virtual data_t take_samples() override;

		CLONE(Simulator, GraphCliffordSimulator)
};