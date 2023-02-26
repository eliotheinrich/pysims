#ifndef QA_CONFIG_H
#define QA_CONFIG_H

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include "QuantumAutomatonSimulator.h"

#define DEFAULT_SIMULATOR "chp"

class QuantumAutomatonConfig : public TimeConfig, public Entropy {
	private:
		float mzr_prob;
		CliffordType simulator_type;

		QuantumAutomatonSimulator *simulator;

		virtual void init_state();
		virtual std::map<std::string, Sample> take_samples();

	public:
		QuantumAutomatonConfig(Params &p);

		// Implementing TimeConfig
		virtual void timesteps(uint num_steps);

		// Implementing Entropy
		virtual float entropy(std::vector<uint> &qubits) const { return simulator->entropy(qubits); }

		static std::vector<QuantumAutomatonConfig*> load_json(nlohmann::json filename);
};

#endif