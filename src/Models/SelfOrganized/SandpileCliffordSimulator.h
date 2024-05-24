#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

class SandpileCliffordSimulator : public dataframe::Simulator {
	private:
		uint32_t system_size;

		double unitary_prob;
		double mzr_prob;

		std::string boundary_condition;
		uint32_t feedback_mode;


		uint32_t unitary_qubits;
		uint32_t mzr_mode;
		
		std::vector<uint32_t> feedback_strategy;

		bool start_sampling;
		bool sample_avalanche_sizes;

    bool sample_reduced_surface;
		
		uint32_t initial_state;
		uint32_t scrambling_steps;

		InterfaceSampler interface_sampler;
		EntropySampler entropy_sampler;

		void feedback(uint32_t q);

		void left_boundary();
		void right_boundary();

		void mzr(uint32_t i);
		void unitary(uint32_t i);
		
		void timestep();

		uint32_t get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const;

    void add_reduced_substrate_height_samples(dataframe::data_t& samples, const std::vector<int>& surface) const;

	public:
		std::shared_ptr<QuantumCHPState> state;

		SandpileCliffordSimulator(dataframe::Params &params, uint32_t);

		virtual void equilibration_timesteps(uint32_t num_steps) override {
			start_sampling = false;
			timesteps(num_steps);
			start_sampling = true;
		}

		virtual void timesteps(uint32_t num_steps) override;

    virtual std::vector<dataframe::byte_t> serialize() const override;
    virtual void deserialize(const std::vector<dataframe::byte_t>& data) override;

		virtual dataframe::data_t take_samples() override;
};
