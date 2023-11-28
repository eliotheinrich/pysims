#pragma once

#include <Simulator.hpp>
#include <QuantumCHPState.hpp>

class PhaselessSimulator : public Simulator {
	private:
		uint32_t system_size;

		double mzr_prob;
		double z_prob;

		uint32_t num_x_eigenstates;
		uint32_t dim;
		
		bool sample_measurement_outcomes;

		std::shared_ptr<QuantumCHPState> state;
		bool offset;

		EntropySampler sampler;

		void measure(uint32_t q);
		void apply_gate(uint32_t q1, uint32_t q2);

	public:
		PhaselessSimulator(Params& params);

		virtual void init_state(uint32_t) override;
		void one_dimensional_timestep();
		void two_dimensional_timestep();
		virtual void timesteps(uint32_t num_steps) override;

		data_t take_samples() override;

		CLONE(Simulator, PhaselessSimulator);
};