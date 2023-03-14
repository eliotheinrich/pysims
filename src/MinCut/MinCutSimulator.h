#ifndef MC_CONFIG
#define MC_CONFIG

#include "Graph.h"
#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include <random>
#include "Entropy.hpp"

class MinCutSimulator : public EntropySimulator {
	private:
		Graph state;

		float mzr_prob;

		bool offset;

	public:
		MinCutSimulator(Params &params);

		std::string to_string() const;

		virtual void init_state();

		virtual void timesteps(uint num_steps);
		virtual float entropy(std::vector<uint> &sites) const;
		virtual std::unique_ptr<Simulator> clone(Params &params) { return std::unique_ptr<Simulator>(new MinCutSimulator(params)); }
};

#endif