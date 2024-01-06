#pragma once

#include <Graph.hpp>
#include <Simulator.hpp>
#include <Samplers.h>

class GraphEntropyState : public EntropyState {
	public:
		Graph state;
		virtual double entropy(const std::vector<uint32_t>& sites, uint32_t index) override;
		GraphEntropyState()=default;
		GraphEntropyState(uint32_t num_nodes) {
			state = Graph(num_nodes);
		}
};

class MinCutSimulator : public dataframe::Simulator {
	private:
		std::shared_ptr<GraphEntropyState> state;

		uint32_t system_size;
		double mzr_prob;

		bool offset;

		EntropySampler sampler;

	public:
		MinCutSimulator(dataframe::Params &params, uint32_t);

		std::string to_string() const;

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::data_t take_samples() override;
};
