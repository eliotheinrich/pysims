#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumCHPState.hpp>

class SandpileCliffordSimulator : public EntropySimulator {
	private:
		std::unique_ptr<QuantumCHPState<Tableau>> state;

		float unitary_prob;
		float mzr_prob;

		uint32_t system_size;

		std::string boundary_condition;
		uint32_t feedback_mode;


		uint32_t unitary_qubits;
		uint32_t mzr_mode;
		
		std::vector<uint32_t> feedback_strategy;

		void feedback(uint32_t q);

		void left_boundary();
		void right_boundary();

		void mzr(uint32_t i);
		void unitary(uint32_t i);
		
		void timestep();

		uint32_t get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const;

	public:
		SandpileCliffordSimulator(Params &params);

		virtual void init_state(uint32_t) override { 
			state = std::make_unique<QuantumCHPState<Tableau>>(system_size);
		}

		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits); }

		virtual void timesteps(uint32_t num_steps) override;
		virtual std::string serialize() const override {
			std::string s = state->to_string();
			auto substrings = split(s, "\n");
			substrings.erase(substrings.begin());
			return join(substrings, "\n");
		}

		virtual std::shared_ptr<Simulator> deserialize(Params &params, const std::string &data) override {
			std::shared_ptr<SandpileCliffordSimulator> sim(new SandpileCliffordSimulator(params));
			sim->state = std::unique_ptr<QuantumCHPState<Tableau>>(
				new QuantumCHPState(data)
			);

			return sim;
		}

		CLONE(Simulator, SandpileCliffordSimulator)
};