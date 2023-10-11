#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumCHPState.hpp>

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
							if (q % system_size != q) periodic = true;
							return q % system_size; 
						});
		
		if (!(!periodic_bc && periodic))
			state->random_clifford(offset_qubits);
	}
}

class RandomCliffordSimulator : public EntropySimulator {
	private:
		std::shared_ptr<CliffordState> state;

		float mzr_prob;
		uint32_t gate_width;

		std::string simulator_type;
		
		bool initial_offset;
		bool periodic_bc;

		bool sample_sparsity;

	public:
		RandomCliffordSimulator(Params &params);

		virtual void init_state(uint32_t) override;
		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits); }
		virtual void timesteps(uint32_t num_steps) override;

		virtual std::string serialize() const override {
			std::string s = state->to_string();
			auto substrings = split(s, "\n");
			substrings.erase(substrings.begin());
			return join(substrings, "\n");
		}

		virtual std::shared_ptr<Simulator> deserialize(Params &params, const std::string &data) override {
			std::shared_ptr<RandomCliffordSimulator> sim(new RandomCliffordSimulator(params));
			sim->state = std::shared_ptr<CliffordState>(
				new QuantumCHPState(data)
			);

			return sim;
		}

		virtual data_t take_samples() override;

		CLONE(Simulator, RandomCliffordSimulator)
};