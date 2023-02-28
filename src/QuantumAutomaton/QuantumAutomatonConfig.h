#ifndef QA_CONFIG_H
#define QA_CONFIG_H

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include "QuantumAutomatonSimulator.h"

#define DEFAULT_CLIFFORD_STATE "chp"

class QuantumAutomatonConfig : public EntropyConfig {
	private:
		float mzr_prob;
		CliffordType simulator_type;

		QuantumAutomatonSimulator *simulator;

		virtual void init_state();

	public:
		QuantumAutomatonConfig(Params &p);

		// Implementing EntropyConfig 
		virtual void timesteps(uint num_steps);
		virtual float entropy(std::vector<uint> &qubits) const { return simulator->entropy(qubits); }

		static std::vector<QuantumAutomatonConfig*> load_json(nlohmann::json filename);
};

#endif