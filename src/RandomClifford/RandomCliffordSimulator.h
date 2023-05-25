#ifndef RC_SIM_H
#define RC_SIM_H

#include <DataFrame.hpp>
#include "Entropy.hpp"
#include "CliffordState.hpp"

inline static void rc_timestep(std::shared_ptr<CliffordState> state, uint gate_width, bool offset_layer) {
	uint system_size = state->system_size();
	uint num_gates = system_size / gate_width;

	std::vector<uint> qubits(gate_width);
	std::iota(qubits.begin(), qubits.end(), 0);

	for (uint j = 0; j < num_gates; j++) {
		uint offset = offset_layer ? gate_width*j : gate_width*j + gate_width/2;


		std::vector<uint> offset_qubits(qubits);
		std::transform(offset_qubits.begin(), offset_qubits.end(), 
						offset_qubits.begin(), [system_size, offset](uint x) { return (x + offset) % system_size; } );
		state->random_clifford(offset_qubits);
	}
}

class RandomCliffordSimulator : public EntropySimulator {
	private:
		std::shared_ptr<CliffordState> state;

		CliffordType clifford_type;
		float mzr_prob;
		uint gate_width;
		
		bool initial_offset;

	public:
		RandomCliffordSimulator(Params &params);

		virtual void init_state() override;

		virtual float entropy(const std::vector<uint> &qubits) const override { return state->entropy(qubits); }
		virtual void timesteps(uint num_steps) override;

		CLONE(Simulator, RandomCliffordSimulator)
};

#endif