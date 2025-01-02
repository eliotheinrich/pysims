#pragma once

#include <Graph.hpp>
#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

class NetworkCliffordSimulator : public Simulator {
	private:
		uint32_t system_size;

		double p;
		double mzr_prob;

		double alpha;

		uint32_t num_partitions;

		std::shared_ptr<QuantumCHPState> state;
		Graph<> network;

		EntropySampler sampler;


		void add_degree_distribution(dataframe::SampleMap& samples) const;
		void add_spatially_averaged_entropy(dataframe::SampleMap& samples);

	public:
		NetworkCliffordSimulator(dataframe::ExperimentParams& params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;

		dataframe::SampleMap take_samples() override;
};
