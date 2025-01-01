#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

inline static void qa_layer(std::shared_ptr<CliffordState> state, bool offset, bool gate_type) {
	uint32_t system_size = state->num_qubits;
	for (uint32_t i = 0; i < system_size/2; i++) {
		uint32_t qubit1 = offset ? (2*i + 1) % system_size : 2*i;
		uint32_t qubit2 = offset ? (2*i + 2) % system_size : (2*i + 1) % system_size;

		if (state->rand() % 2 == 0) {
			std::swap(qubit1, qubit2);
		}

		if (gate_type) {
			state->cz(qubit1, qubit2);
		} else {
			state->cx(qubit1, qubit2);
		}
	}
}

inline static void qa_timestep(std::shared_ptr<CliffordState> state) {
	qa_layer(state, false, false); // no offset, cx
	qa_layer(state, false, true);  // no offset, cz
	qa_layer(state, true, false);  // offset,    cx
	qa_layer(state, true, true);   // offset,    cz
}

static inline double qa_power_law(double x0, double x1, double n, double r) {
	return std::pow(((std::pow(x1, n + 1.0) - std::pow(x0, n + 1.0))*r + std::pow(x0, n + 1.0)), 1.0/(n + 1.0));
}

class QuantumAutomatonSimulator : public Simulator {
	private:
		uint32_t system_size;

		std::shared_ptr<CliffordState> state;
		CliffordType clifford_type;
		double mzr_prob;

		uint32_t timestep_type;
		double alpha;

		bool sample_surface;

		EntropySampler entropy_sampler;
		InterfaceSampler interface_sampler;

		uint32_t randpl();

		void timesteps_powerlaw(uint32_t);
		void timesteps_brickwork(uint32_t);

	public:
		QuantumAutomatonSimulator(dataframe::ExperimentParams &params, uint32_t);

		virtual void timesteps(uint32_t num_steps) override;

		virtual dataframe::data_t take_samples() override;
};
