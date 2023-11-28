#pragma once

#include <Graph.h>
#include <Simulator.hpp>
#include <QuantumCHPState.hpp>

class NetworkCliffordSimulator : public Simulator {
	private:
		uint32_t system_size;

		double p;
		double mzr_prob;

		double alpha;

		uint32_t num_partitions;

		std::shared_ptr<QuantumCHPState> state;
		Graph network;

		EntropySampler sampler;


		void add_degree_distribution(data_t& samples) const;
		void add_spatially_averaged_entropy(data_t& samples);

	public:
		NetworkCliffordSimulator(Params& params);

		virtual void init_state(uint32_t) override;
		virtual void timesteps(uint32_t num_steps) override;

		data_t take_samples() override;

		CLONE(Simulator, NetworkCliffordSimulator);
};