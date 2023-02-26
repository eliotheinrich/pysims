#ifndef RC_CONFIG_H
#define RC_CONFIG_H

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include "RandomCliffordSimulator.h"

#define DEFAULT_SIMULATOR "chp"

class RandomCliffordConfig : public TimeConfig, public Entropy {
	private:
		uint gate_width;
		float mzr_prob;
		CliffordType simulator_type;

		RandomCliffordSimulator *simulator;

		virtual void init_state();
		virtual std::map<std::string, Sample> take_samples();


	public:
		RandomCliffordConfig(Params &p);

		// Implementing TimeConfig
		virtual void timesteps(uint num_steps);

		// Implementing Entropy 
		virtual float entropy(std::vector<uint> &qubits) const { return simulator->entropy(qubits); }

		static std::vector<RandomCliffordConfig*> load_json(nlohmann::json data);
};

#endif