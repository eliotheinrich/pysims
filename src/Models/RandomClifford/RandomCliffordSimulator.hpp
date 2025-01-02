#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

#include <Display.h>

#include <glaze/glaze.hpp>

#define RC_DEFAULT_GATE_WIDTH 2

#define RC_DEFAULT_CLIFFORD_SIMULATOR "chp"

#define RC_DEFAULT_PBC true

#define RC_BRICKWORK 0
#define RC_RANDOM_LOCAL 1
#define RC_RANDOM_NONLOCAL 2
#define RC_POWERLAW 3


inline static void rc_timestep(std::shared_ptr<CliffordState> state, uint32_t gate_width, bool offset_layer, bool periodic_bc = true) {
	uint32_t system_size = state->num_qubits;
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

static inline double rc_power_law(double x0, double x1, double n, double r) {
	return std::pow(((std::pow(x1, n + 1.0) - std::pow(x0, n + 1.0))*r + std::pow(x0, n + 1.0)), 1.0/(n + 1.0));
}

static Color RC_COLOR1 = {246.0/255, 77.0/255, 77.0/255, 1.0};
static Color RC_COLOR2 = {77.0/255, 105.0/255, 246.0/255, 1.0};
static Color RC_COLOR3 = {189.0/255, 74.0/255, 218.0/255, 1.0};

class RandomCliffordSimulator : public Simulator {
	private:
		int seed;

		uint32_t system_size;
		double mzr_prob;
		uint32_t gate_width;
		uint32_t timestep_type;
		double alpha;

		std::string simulator_type;
		
		bool offset;
		bool pbc;

		bool sample_sparsity;
		bool sample_avalanche_sizes;

		EntropySampler entropy_sampler;
		InterfaceSampler interface_sampler;
		bool start_sampling;

    uint32_t randpl() {
      return rc_power_law(1.0, system_size/2.0, -alpha, randf()); 
    }

    void timestep_random_local() {
      uint32_t q1 = rand() % system_size;
      uint32_t q2 = (q1 + 1) % system_size;

      std::vector<uint32_t> qbits{q1, q2};;
      state->random_clifford(qbits);

      if (randf() < mzr_prob) {
        mzr(rand() % system_size);
      }
    }

    void timestep_random_nonlocal() {
      uint32_t q1 = rand() % system_size;
      uint32_t q2 = rand() % system_size;
      while (q2 == q1) {
        q2 = rand() % system_size;
      }

      std::vector<uint32_t> qbits{q1, q2};;
      state->random_clifford(qbits);

      if (randf() < mzr_prob) {
        mzr(rand() % system_size);
      }
    }

    void timestep_powerlaw() {
      uint32_t q1 = rand() % system_size;
      uint32_t dq = randpl();
      uint32_t q2 = (rand() % 2) ? mod(q1 + dq, system_size) : mod(q1 - dq, system_size);

      std::vector<uint32_t> qbits{q1, q2};;
      state->random_clifford(qbits);

      if (randf() < mzr_prob) {
        mzr(rand() % system_size);
      }
    }

    void timestep_brickwork(uint32_t num_steps) {
      if (system_size % gate_width != 0) {
        throw std::invalid_argument("Invalid gate width. Must divide system size.");
      } if (gate_width % 2 != 0) {
        throw std::invalid_argument("Gate width must be even.");
      }

      for (uint32_t i = 0; i < num_steps; i++) {
        rc_timestep(state, gate_width, offset, pbc);

        // Apply measurements
        for (uint32_t j = 0; j < system_size; j++) {
          if (state->randf() < mzr_prob) {
            mzr(j);
          }
        }

        offset = !offset;
      }
    }

    void mzr(uint32_t q) {
      if (sample_avalanche_sizes && start_sampling) {
        std::vector<int> surface1 = state->get_entropy_surface<int>(2);
        state->mzr(q);
        std::vector<int> surface2 = state->get_entropy_surface<int>(2);

        int s = 0.0;
        for (uint32_t i = 0; i < system_size; i++) {
          s += std::abs(surface1[i] - surface2[i]);
        }

        interface_sampler.record_size(s);
      } else {
        state->mzr(q);
      }
    }

	public:
		std::shared_ptr<QuantumCHPState> state;

		RandomCliffordSimulator(dataframe::ExperimentParams &params, uint32_t num_threads) : Simulator(params), entropy_sampler(params), interface_sampler(params) {
      system_size = dataframe::utils::get<int>(params, "system_size");

      mzr_prob = dataframe::utils::get<double>(params, "mzr_prob");
      gate_width = dataframe::utils::get<int>(params, "gate_width", RC_DEFAULT_GATE_WIDTH);

      timestep_type = dataframe::utils::get<int>(params, "timestep_type", RC_BRICKWORK);
      if (timestep_type == RC_POWERLAW) {
        alpha = dataframe::utils::get<double>(params, "alpha");
      }

      simulator_type = dataframe::utils::get<std::string>(params, "simulator_type", RC_DEFAULT_CLIFFORD_SIMULATOR);

      sample_avalanche_sizes = dataframe::utils::get<int>(params, "sample_avalanche_sizes", false);

      offset = false;
      pbc = dataframe::utils::get<int>(params, "pbc", RC_DEFAULT_PBC);

      sample_sparsity = dataframe::utils::get<int>(params, "sample_sparsity", false);	

      seed = dataframe::utils::get<int>(params, "seed", -1);

      start_sampling = false;


      state = std::make_shared<QuantumCHPState>(system_size, seed);
    }

		virtual void equilibration_timesteps(uint32_t num_steps) override {
			start_sampling = false;
			timesteps(num_steps);
			start_sampling = true;
		}

    virtual void timesteps(uint32_t num_steps) override {
      if (timestep_type == RC_BRICKWORK) {
        timestep_brickwork(num_steps);
      } else if (timestep_type == RC_RANDOM_LOCAL) {
        timestep_random_local();
      } else if (timestep_type == RC_RANDOM_NONLOCAL) {
        timestep_random_nonlocal();
      } else if (timestep_type == RC_POWERLAW) {
        timestep_powerlaw();
      }

      state->tableau.rref();
    }

    virtual std::vector<dataframe::byte_t> serialize() const override {
      std::vector<dataframe::byte_t> data;
      auto write_error = glz::write_beve(*this, data);
      if (write_error) {
        throw std::runtime_error(fmt::format("Error serializing RandomCliffordSimulator: \n{}", glz::format_error(write_error, data)));
      }
      return data;
    }

    virtual void deserialize(const std::vector<dataframe::byte_t>& bytes) override {
      auto parse_error = glz::read_beve(*this, bytes);
      if (parse_error) {
        throw std::runtime_error(fmt::format("Error deserializing RandomCliffordSimulator: \n{}", glz::format_error(parse_error, bytes)));
      }
    }

    virtual dataframe::SampleMap take_samples() override {
      dataframe::SampleMap samples;

      entropy_sampler.add_samples(samples, state);

      std::vector<int> surface = state->get_entropy_surface<int>(2);
      interface_sampler.add_samples(samples, surface);

      if (sample_sparsity) {
        samples.emplace("sparsity", state->sparsity());
      }

      return samples;
    }

    virtual Texture get_texture() const override {
      return state->get_texture(RC_COLOR1, RC_COLOR2, RC_COLOR3);
    }

    struct glaze {
      static constexpr auto value = glz::object(
          "state", &RandomCliffordSimulator::state
      );
    };
};

