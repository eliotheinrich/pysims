#pragma once

#include <Simulator.hpp>
#include <CliffordState.hpp>
#include <InterfaceSampler.hpp>

inline static void rc_timestep(std::shared_ptr<CliffordState> state, uint32_t gate_width, bool offset_layer, bool periodic_bc = true) {
	uint32_t system_size = state->system_size();
	uint32_t num_gates = system_size / gate_width;

	std::vector<uint32_t> qubits(gate_width);
	std::iota(qubits.begin(), qubits.end(), 0);

	for (uint32_t j = 0; j < num_gates; j++) {
		uint32_t offset = offset_layer ? gate_width*j : gate_width*j + gate_width/2;

		bool periodic = false;
		std::vector<uint32_t> offset_qubits(qubits);
		std::transform(offset_qubits.begin(), offset_qubits.end(), offset_qubits.begin(), 
						[system_size, offset, &periodic](uint32_t x) { 
							uint32_t q = x + offset;
							if (q % system_size != q) {
								periodic = true;
							}
							return q % system_size; 
						});
		
		if (!(!periodic_bc && periodic)) {
			state->random_clifford(offset_qubits);
		}
	}
}

class RandomCliffordSimulator : public Simulator {
	private:
		std::shared_ptr<CliffordState> state;
		int seed;

		uint32_t system_size;
		double mzr_prob;
		uint32_t gate_width;

		std::string simulator_type;
		
		bool initial_offset;
		bool pbc;

		bool sample_sparsity;
		bool sample_avalanche_sizes;

		EntropySampler entropy_sampler;
		InterfaceSampler interface_sampler;
		bool start_sampling;

	public:
		RandomCliffordSimulator(Params &params, uint32_t num_threads);

		virtual void equilibration_timesteps(uint32_t num_steps) override {
			start_sampling = false;
			timesteps(num_steps);
			start_sampling = true;
		}
		virtual void timesteps(uint32_t num_steps) override;

		virtual data_t take_samples() override;
};