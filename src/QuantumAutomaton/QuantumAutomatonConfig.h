#ifndef QA_CONFIG_H
#define QA_CONFIG_H

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include "QuantumAutomatonSimulator.h"

#define DEFAULT_NUM_RUNS 1u
#define DEFAULT_EQUILIBRATION_STEPS 0u
#define DEFAULT_SAMPLING_TIMESTEPS 0u
#define DEFAULT_MEASUREMENT_FREQ 1u
#define DEFAULT_SPACING 1u
#define DEFAULT_SIMULATOR "chp"
#define DEFAULT_TEMPORAL_AVG true

class QuantumAutomatonConfig : public TimeConfig, public Entropy {
	private:
		float mzr_prob;
		CliffordType simulator_type;

		QuantumAutomatonSimulator *simulator;

		virtual void init_state();
		virtual std::map<std::string, Sample> take_samples();

	public:
		QuantumAutomatonConfig(std::map<std::string, int> iparams, std::map<std::string, float> fparams);


		// Implementing TimeConfig
		virtual void timesteps(uint num_steps);
		virtual std::map<std::string, int> get_iparams() const;
		virtual std::map<std::string, float> get_fparams() const;

		// Implementing Entropy
		virtual float entropy(std::vector<uint> &qubits) const { return simulator->entropy(qubits); }

		static std::vector<QuantumAutomatonConfig*> load_json(nlohmann::json filename);
};

#endif