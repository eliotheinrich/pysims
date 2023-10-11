#pragma once

#include <Graph.h>
#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include <random>
#include <Entropy.hpp>

class MinCutSimulator : public EntropySimulator {
	private:
		Graph state;

		float mzr_prob;

		bool offset;

	public:
		MinCutSimulator(Params &params);

		std::string to_string() const;

		virtual void init_state(uint32_t) override {
			state = Graph(system_size/2);
		}

		virtual void timesteps(uint32_t num_steps) override;
		virtual double entropy(const std::vector<uint32_t> &sites, uint32_t index) const override;

		CLONE(Simulator, MinCutSimulator)
};