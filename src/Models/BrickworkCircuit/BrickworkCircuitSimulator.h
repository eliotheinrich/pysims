#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumState.h>



class BrickworkCircuitSimulator : public EntropySimulator {
	private:
		float mzr_prob;
		int gate_type;

		bool offset;

		void mzr(uint32_t q);

	public:
		std::shared_ptr<Statevector> state;

		BrickworkCircuitSimulator(Params &params);

		virtual void init_state(uint32_t num_threads) override {
			Eigen::setNbThreads(num_threads);
			state = std::make_shared<Statevector>(system_size);
		}

		virtual double entropy(const std::vector<uint32_t> &qubits, uint index) const override { return state->entropy(qubits, index); }
		virtual void timesteps(uint32_t num_steps) override;

		CLONE(Simulator, BrickworkCircuitSimulator)
};