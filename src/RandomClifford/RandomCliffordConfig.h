#ifndef RC_CONFIG_H
#define RC_CONFIG_H

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>
#include "RandomCliffordSimulator.h"

#define DEFAULT_CLIFFORD_STATE "chp"
#define DEFAULT_DRIVE_TYPE "undriven"

enum DriveType { Undriven, Test };

static DriveType parse_drive_type(std::string s) {
	if (s == "undriven") return DriveType::Undriven;
	else if (s == "test") return DriveType::Test;
	else {
		std::cout << "Drive type " << s << " not supported. Defaulting to " << DEFAULT_DRIVE_TYPE << "\n";
		return parse_drive_type(DEFAULT_DRIVE_TYPE);
	}
}

class RandomCliffordConfig : public EntropyConfig {
	private:
		uint gate_width;
		float init_mzr_prob;
		CliffordType clifford_state;
		DriveType drive_type;

		RandomCliffordSimulator *simulator;

		virtual void init_state();

	public:
		RandomCliffordConfig(Params &p);

		// Implementing EntropyConfig
		virtual void timesteps(uint num_steps);
		virtual float entropy(std::vector<uint> &qubits) const { return simulator->entropy(qubits); }

		static std::vector<RandomCliffordConfig*> load_json(nlohmann::json data);
};

#endif