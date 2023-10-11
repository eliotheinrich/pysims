#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumCHPState.hpp>

class PhaselessSimulator : public EntropySimulator {
	private:
		double mzr_prob;
		double z_prob;

		uint32_t num_x_eigenstates;
		uint32_t dim;
		
		bool sample_measurement_outcomes;

		std::unique_ptr<QuantumCHPState<Tableau>> state;
		bool offset;

	public:
		PhaselessSimulator(Params& params);

		virtual void init_state(uint32_t) override;
		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits); }
		void one_dimensional_timestep();
		void two_dimensional_timestep();
		virtual void timesteps(uint32_t num_steps) override;

		data_t take_samples() override;
};