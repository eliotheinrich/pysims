#ifndef SANDPILE_CLIFFORD_SIM_H
#define SANDPILE_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

#define DEFAULT_RANDOM_SITES true
#define DEFAULT_BOUNDARY_CONDITIONS 0
#define DEFAULT_FEEDBACK_MODE 0

class SandpileCliffordSimulator : public Simulator {
	private:
		std::unique_ptr<QuantumCHPState> state;

		float unitary_prob;
		float mzr_prob;

		uint system_size;

		bool random_sites;
		int boundary_conditions;
		int feedback_mode;

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