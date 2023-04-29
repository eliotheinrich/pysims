#ifndef TWO_SPECIES_SIM_H
#define TWO_SPECIES_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "QuantumCHPState.h"

class TwoSpeciesSimulator : public EntropySimulator {
	private:
		std::unique_ptr<QuantumCHPState> state;

		std::vector<int> occupation;
		int initial_state;

		bool random_sites;

		uint get_shape(uint s0, uint s1, uint s2) const;

	public:
		TwoSpeciesSimulator(Params &params);

		virtual void init_state() override;

		virtual float entropy(std::vector<uint> &qubits) const override { return state->entropy(qubits); }

		void timestep();
		virtual void timesteps(uint num_steps) override;

		virtual data_t take_samples() override;

		CLONE(Simulator, TwoSpeciesSimulator)
};

#endif