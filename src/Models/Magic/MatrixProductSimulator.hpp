#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

#define MPSS_PROJECTIVE 0
#define MPSS_WEAK 1

#define MPSS_HAAR 0
#define MPSS_CLIFFORD 1

class MatrixProductSimulator : public dataframe::Simulator {
	private:
		uint32_t system_size;
		double beta;
    double xx_prob;
    size_t bond_dimension;

    int measurement_type;
    int unitary_type;

		InterfaceSampler interface_sampler;
    QuantumStateSampler quantum_sampler;

    bool offset;

    void measure(size_t i, size_t j) {
      // TODO check that single-qubit and two-qubit measurements are balanced

      if (measurement_type == MPSS_PROJECTIVE) {
        // Do projective measurement
        if (randf() < beta) {
          if (randf() < xx_prob) {
            PauliString XX(2);
            XX.set_x(0, 1);
            XX.set_x(1, 1);
            state->measure(XX, {static_cast<uint32_t>(i), static_cast<uint32_t>(j)});
          } else {
            state->mzr(i);
          }
        }
      } else if (measurement_type == MPSS_WEAK) {
        // Do weak measurement
        if (randf() < xx_prob) {
          PauliString XX(2);
          XX.set_x(0, 1);
          XX.set_x(1, 1);
          state->weak_measure(XX, {static_cast<uint32_t>(i), static_cast<uint32_t>(j)}, beta);
        } else {
          PauliString Z(1);
          Z.set_z(0, 1);
          state->weak_measure(Z, {static_cast<uint32_t>(i)}, beta);
        }
      }

    }

    void unitary(uint32_t i, uint32_t j) {
      if (unitary_type == MPSS_HAAR) {
        Eigen::Matrix4cd gate = haar_unitary(2, rng);
        state->evolve(gate, {i, j});
      } else if (unitary_type == MPSS_CLIFFORD) {
        state->random_clifford({i, j});
      } else {
        throw std::runtime_error(fmt::format("Invalid unitary type {}.", unitary_type));
      }
    }

	public:
    std::shared_ptr<MatrixProductState> state;
		MatrixProductSimulator(dataframe::Params &params, uint32_t num_threads) : Simulator(params), interface_sampler(params), quantum_sampler(params){
      system_size = dataframe::utils::get<int>(params, "system_size");
      beta = dataframe::utils::get<double>(params, "beta");
      xx_prob = dataframe::utils::get<double>(params, "xx_prob");
      bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 32);

      measurement_type = dataframe::utils::get<int>(params, "measurement_type", MPSS_PROJECTIVE);
      unitary_type = dataframe::utils::get<int>(params, "unitary_type", MPSS_HAAR);

      offset = false;

      state = std::make_shared<MatrixProductState>(system_size, bond_dimension);
      state->seed(rand());

      PauliMutationFunc bit_mutation = [](PauliString& p, std::minstd_rand& rng) {
        size_t j = rng() % (2*p.num_qubits);
        p.set(j, !p.get(j));
      };

      quantum_sampler.set_montecarlo_update(bit_mutation);
    }

		virtual void timesteps(uint32_t num_steps) override {
      for (size_t t = 0; t < num_steps; t++) {
        for (size_t i = 0; i < system_size/2 - offset; i++) {
          size_t q1 = 2*i + offset;
          size_t q2 = 2*i + 1 + offset;
          unitary(q1, q2);
        }

        for (size_t i = 0; i < system_size-1; i++) {
          // do measurements
          measure(i, i+1);
        }

        // Maybe mzr rightmost edge
        if (randf() < beta && randf() > xx_prob) {
          state->mzr(system_size - 1);
        }

        offset = !offset;
      }
    }

		virtual dataframe::data_t take_samples() override {
      dataframe::data_t samples;

      std::vector<int> surface = state->get_entropy_surface<int>(1u);
      interface_sampler.add_samples(samples, surface);

      quantum_sampler.add_samples(samples, state);

      double trace = state->trace();
      dataframe::utils::emplace(samples, "trace", trace);

      std::vector<double> bond_dimensions(system_size - 1);
      for (size_t i = 0; i < system_size - 1; i++) {
        bond_dimensions[i] = static_cast<double>(state->bond_dimension(i));
      }
      dataframe::utils::emplace(samples, "bond_dimension_at_site", bond_dimensions);

      return samples;
    }
};
