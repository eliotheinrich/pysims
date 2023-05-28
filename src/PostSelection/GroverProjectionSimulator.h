#ifndef GP_SIM_H
#define GP_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "UnitaryState.hpp"

class GroverProjectionSimulator : public EntropySimulator {
	private:
		std::minstd_rand rng;
		float mzr_prob;
		uint nmax;

		bool offset;

	public:
		std::shared_ptr<UnitaryState> state;
		void grover_projection(uint qubit);
		void grover_projection(uint qubit, bool outcome);

		GroverProjectionSimulator(Params &params);

		virtual void init_state() override;

		virtual float entropy(const std::vector<uint> &qubits, uint index) const override { return state->entropy(qubits, index); }
		virtual void timesteps(uint num_steps) override;

		CLONE(Simulator, GroverProjectionSimulator)
};

#endif