#ifndef SOC_CLIFFORD_SIM_H
#define SOC_CLIFFORD_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

#define DEFAULT_RANDOM_SITES true
#define DEFAULT_BOUNDARY_CONDITIONS 0

class SelfOrganizedCliffordSimulator : public Simulator {
	private:
		std::unique_ptr<QuantumCHPState> state;

		float unitary_prob;
		float mzr_prob;

		uint system_size;

		bool random_sites;
		int boundary_conditions;

		int cum_entropy(uint i) const;

	public:
		SelfOrganizedCliffordSimulator(Params &params);

		virtual void init_state() { 
			state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(system_size)); 
		}

		virtual std::map<std::string, Sample> take_samples();

		void mzr(uint i);
		void unitary(uint i);
		
		void timestep();
		virtual void timesteps(uint num_steps);

		CLONE(Simulator, SelfOrganizedCliffordSimulator)
};

#endif