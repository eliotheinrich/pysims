#ifndef MC_CONFIG
#define MC_CONFIG

#include "Graph.h"
#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include <random>
#include "Simulator.hpp"

class MinCutSimulator : public EntropySimulator {
	private:
		std::minstd_rand *rng;
		Graph *state;

		float mzr_prob;

		bool offset;

	public:
		virtual void init_state();

		MinCutSimulator(Params &params);
		~MinCutSimulator();

		int rand() { return (*rng)(); }
		float randf() { return double((*rng)())/double(RAND_MAX); }

		virtual void timesteps(uint num_steps);
		virtual float entropy(std::vector<uint> &sites) const;
};

static const uint mod(int a, int b) {
	int c = a % b;
	return (c < 0) ? c + b : c;
}

#endif