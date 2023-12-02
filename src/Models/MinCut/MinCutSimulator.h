#pragma once

#include <Graph.h>
#include <EntropySampler.hpp>
#include <Simulator.hpp>
#include <nlohmann/json.hpp>

class GraphEntropyState : public EntropyState {
	public:
		Graph state;
		virtual double entropy(const std::vector<uint32_t>& sites, uint32_t index) override;
		GraphEntropyState()=default;
		GraphEntropyState(uint32_t num_nodes) {
			state = Graph(num_nodes);
		}
};

class MinCutSimulator : public Simulator {
	private:
		std::shared_ptr<GraphEntropyState> state;

		uint32_t system_size;
		double mzr_prob;

		bool offset;

		EntropySampler sampler;

	public:
		MinCutSimulator(Params &params);

		std::string to_string() const;

		virtual void init_state(uint32_t) override {
			state = std::make_shared<GraphEntropyState>(system_size/2);
		}

		virtual void timesteps(uint32_t num_steps) override;

		virtual data_t take_samples() override;

		CLONE(Simulator, MinCutSimulator)
};