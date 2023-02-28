#ifndef MC_CONFIG
#define MC_CONFIG

#include "Graph.h"
#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include <random>
#include "Simulator.hpp"

#define DEFAULT_NUM_RUNS 1u
#define DEFAULT_EQUILIBRATION_STEPS 0u
#define DEFAULT_SAMPLING_TIMESTEPS 0u
#define DEFAULT_MEASUREMENT_FREQ 1u
#define DEFAULT_SPACING 1u
#define DEFAULT_TEMPORAL_AVG true

class MinCutConfig : public EntropyConfig {
	private:
		std::minstd_rand *rng;
		Graph *state;

		float mzr_prob;

		bool offset;

		virtual void init_state();

	public:
		MinCutConfig(Params &params);

		int rand() { return (*rng)(); }
		float randf() { return double((*rng)())/double(RAND_MAX); }

		// Implementing EntropyConfig
		virtual void timesteps(uint num_steps);
		virtual float entropy(std::vector<uint> &sites) const;

		static std::vector<MinCutConfig*> load_json(nlohmann::json data);

};

static const uint mod(int a, int b) {
	int c = a % b;
	return (c < 0) ? c + b : c;
}

#endif