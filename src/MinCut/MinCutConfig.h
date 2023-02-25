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

class MinCutConfig : public TimeConfig, public Entropy {
	private:
		std::minstd_rand *rng;
		Graph *state;

		float mzr_prob;

		bool offset;

		virtual void init_state();
		virtual std::map<std::string, Sample> take_samples();

	public:
		MinCutConfig(std::map<std::string, int> iparams, std::map<std::string, float> fparams);

		int rand() { return (*rng)(); }
		float randf() { return double((*rng)())/double(RAND_MAX); }

		// Implementing TimeConfig
		virtual void timesteps(uint num_steps);

		// Implementing Entropy
		virtual float entropy(std::vector<uint> &sites) const;

		static std::vector<MinCutConfig*> load_json(nlohmann::json data);

};

#endif