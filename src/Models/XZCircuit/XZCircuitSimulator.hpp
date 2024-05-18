#pragma once

#include <Simulator.hpp>
#include <CliffordState.h>
#include <Samplers.h>

#define XZCIRCUIT_1D 0
#define XZCIRCUIT_1DC 1
#define XZCIRCUIT_2D 2

#define XZCIRCUIT_BASIS 0
#define XZCIRCUIT_SUPERPOS 1
#define XZCIRCUIT_VOLUMELAW 2

class XZCircuitSimulator : public dataframe::Simulator {
	private:
    std::shared_ptr<QuantumCHPState> state;

		uint32_t system_size;
		double mzr_prob;
    double pz;

    uint32_t initial_state;
    uint32_t dim;

    bool random_sites;


		EntropySampler entropy_sampler;
		InterfaceSampler interface_sampler;

    size_t num_qubits() {
      if (dim == XZCIRCUIT_1D) {
        return system_size;
      } else if (dim == XZCIRCUIT_1DC) {
        return 2*system_size;
      } else if (dim == XZCIRCUIT_2D) {
        return system_size*system_size;
      } else {
        return 0; // unreachable
      }
    }

    void cx_gate(size_t q1, size_t q2) {
      if (rand() % 2) {
        std::swap(q1, q2);
      }

      state->cx(q1, q2);
    }

    void measurements() {
      for (size_t i = 0; i < num_qubits(); i++) {
        if (randf() < mzr_prob) {
          if (randf() < pz) {
            state->mzr(i);
          } else {
            state->mxr(i);
          }
        }
      }
    }

    void timesteps_1d(uint32_t num_steps) {
      for (size_t t = 0; t < num_steps; t++) {
        for (size_t i = 0; i < num_qubits(); i++) {
          size_t q1 = random_sites ? (rand() % system_size) : i;
          size_t q2 = (q1 + 1) % system_size;

          cx_gate(q1, q2);
        }

        measurements();
      }
    }

    void timesteps_1dc(uint32_t num_steps) {
      for (size_t t = 0; t < num_steps; t++) {
        for (size_t i = 0; i < 3*system_size; i++) {
          size_t q1, q2;
          if (random_sites) {
            q1 = rand() % system_size + system_size * (rand() % 2);
            if (q1 < system_size) { // first row
              q2 = (rand() % 2) ? q1 + system_size : (q1 + 1) % system_size;
            } else {
              q2 = (q1 + 1) % system_size;
            }
          } else {
            if (i >= 2*system_size) {
              q1 = i - 2*system_size;
              q2 = q1 + system_size;
            } else if (i >= system_size) {
              q1 = i % system_size + system_size;
              q2 = (i + 1) % system_size + system_size;
            } else {
              q1 = i;
              q2 = (i + 1) % system_size;
            }
          }

          cx_gate(q1, q2);
        }

        measurements();
      }
    }

    void timesteps_2d(uint32_t num_steps) {
      for (size_t t = 0; t < num_steps; t++) {
        for (size_t d = 0; d < 2; d++) {
          for (int x = 0; x < system_size; x++) {
            for (int y = 0; y < system_size; y++) {
              int x1, x2, y1, y2;
              if (random_sites) {
                x1 = rand() % system_size;
                y1 = rand() % system_size;

                if (rand() % 2) {
                  x2 = (x1 + 1) % system_size;
                  y2 = y1;
                } else {
                  x2 = x1;
                  y2 = (y1 + 1) % system_size;
                }
              } else {
                x1 = x;
                y1 = y;
                if (d == 0) {
                  x2 = x1;
                  y2 = (y1 + 1) % system_size;
                } else {
                  x2 = (x1 + 1) % system_size;
                  y2 = y1;
                }
              } 

              size_t q1 = x1 + system_size * y1;
              size_t q2 = x2 + system_size * y2;

              cx_gate(q1, q2);
            }
          }
        }

        measurements();
      }
    }

    void prepare_basis_state() {
      state = std::make_shared<QuantumCHPState>(num_qubits());
    }

    void prepare_superposition_state() {
      state = std::make_shared<QuantumCHPState>(num_qubits());
      for (size_t i = 0; i < num_qubits(); i++) {
        state->h(i);
      }
    }

    void prepare_volume_law_state() {
      state = std::make_shared<QuantumCHPState>(num_qubits());
      std::vector<uint32_t> qubits(system_size);
      std::iota(qubits.begin(), qubits.end(), 0);
      state->random_clifford(qubits);
    }

	public:
		XZCircuitSimulator(dataframe::Params &params, uint32_t) : dataframe::Simulator(params), entropy_sampler(params), interface_sampler(params) {
      system_size = dataframe::utils::get<int>(params, "system_size");
      mzr_prob = dataframe::utils::get<double>(params, "mzr_prob");
      pz = dataframe::utils::get<double>(params, "pz");

      initial_state = dataframe::utils::get<int>(params, "initial_state", XZCIRCUIT_BASIS);
      dim = dataframe::utils::get<int>(params, "dim", XZCIRCUIT_1D);

      random_sites = dataframe::utils::get<int>(params, "random_sites", true);

      if (initial_state == XZCIRCUIT_BASIS) {
        prepare_basis_state();
      } else if (initial_state == XZCIRCUIT_SUPERPOS) {
        prepare_superposition_state();
      } else if (initial_state == XZCIRCUIT_VOLUMELAW) {
        prepare_volume_law_state();
      }
    }

		virtual void timesteps(uint32_t num_steps) override {
      if (dim == XZCIRCUIT_1D) {
        timesteps_1d(num_steps);
      } else if (dim == XZCIRCUIT_1DC) {
        timesteps_1dc(num_steps);
      } else if (dim == XZCIRCUIT_2D) {
        timesteps_2d(num_steps);
      }
    }

		virtual dataframe::data_t take_samples() override {
      dataframe::data_t samples;

      entropy_sampler.add_samples(samples, state);

      std::vector<int> surface = state->get_entropy_surface<int>(2);
      interface_sampler.add_samples(samples, surface);

      return samples;
    }
};
