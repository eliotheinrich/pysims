#ifndef GS_CLIFFORD_SIM_H
#define GS_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumGraphState.h"

class GraphCliffordSimulator : public EntropySimulator {
	private:
		std::string evolution_type;

		std::shared_ptr<QuantumGraphState> state;
		float mzr_prob;

		uint gate_width;
		bool initial_offset;

		uint m;
		float a;

		std::minstd_rand rng;

		void mzr(uint q);

		void unitary_timesteps(uint num_steps);
		void qa_timesteps(uint num_steps);
		void rc_timesteps(uint num_steps);
		void generate_random_graph();

		uint dist(int i, int j) const;
		void add_distance_distribution(data_t &samples) const;
		void add_degree_distribution(data_t &samples) const;
		void add_avg_max_dist(data_t &samples) const;


	public:
		GraphCliffordSimulator(Params &params);

		virtual void init_state() override;

		virtual float entropy(const std::vector<uint> &qubits) const override { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps) override;
		virtual data_t take_samples() override;

		CLONE(Simulator, GraphCliffordSimulator)
};

#endif