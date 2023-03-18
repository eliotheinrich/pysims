#ifndef SANDPILE_CLIFFORD_SIM_H
#define SANDPILE_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

#define DEFAULT_RANDOM_SITES true

enum BoundaryCondition {
	Periodic,
	Open1,
	Open2
};

static BoundaryCondition parse_boundary_condition(std::string s) {
	if (s == "pbc") return BoundaryCondition::Periodic;
	else if (s == "obc1") return BoundaryCondition::Open1;
	else if (s == "obc2") return BoundaryCondition::Open2;
	else {
		std::cout << "Invalid boundary condition: " << s << std::endl;
		assert(false);
	}
}

enum FeedbackMode {
	Mode1,
	Mode2,
	Mode3
};

static FeedbackMode parse_feedback_mode(std::string s) {
	if (s == "mode1") return FeedbackMode::Mode1;
	else if (s == "mode2") return FeedbackMode::Mode2;
	else if (s == "mode3") return FeedbackMode::Mode3;
	else {
		std::cout << "Invalid feedback mode: " << s << std::endl;
		assert(false);
	}
}

#define DEFAULT_BOUNDARY_CONDITIONS "pbc"
#define DEFAULT_FEEDBACK_MODE "mode1"

class SandpileCliffordSimulator : public Simulator {
	private:
		std::unique_ptr<QuantumCHPState> state;

		float unitary_prob;
		float mzr_prob;

		uint system_size;

		bool random_sites;
		BoundaryCondition boundary_condition;
		FeedbackMode feedback_mode;

		int cum_entropy(uint i) const;

		void feedback(int ds1, int ds2, uint q);

		void left_boundary();
		void right_boundary();

		void mzr(uint i);
		void unitary(uint i);
		
		void timestep();

	public:
		SandpileCliffordSimulator(Params &params);

		virtual void init_state() { 
			state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(system_size)); 
		}

		virtual std::map<std::string, Sample> take_samples();

		virtual void timesteps(uint num_steps);

		CLONE(Simulator, SandpileCliffordSimulator)
};

#endif