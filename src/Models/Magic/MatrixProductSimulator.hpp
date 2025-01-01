#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

#define MPSS_PROJECTIVE 0
#define MPSS_WEAK 1

#define MPSS_HAAR 0
#define MPSS_CLIFFORD 1
#define MPSS_Z2_CLIFFORD 2

#define TWO_QUBIT_PAULI PauliString("+XX")
#define ONE_QUBIT_PAULI PauliString("+Z")

static CliffordTable get_z2_table() {
  auto symm = [](const QuantumCircuit& qc) {
    PauliString p = PauliString("+ZZ");
    PauliString p_ = p;
    qc.apply(p);
    return p == p_;
  };

  return CliffordTable(symm);
}

class MatrixProductSimulator : public Simulator {
	private:
		uint32_t system_size;
		double beta;
    double p;
    size_t bond_dimension;

    int measurement_type;
    int unitary_type;

    QuantumStateSampler quantum_sampler;

    CliffordTable z2_table;

    bool offset;

    void measurement_layer() {
      if (measurement_type == MPSS_PROJECTIVE) {
        std::vector<MeasurementData> measurements;
        for (uint32_t i = 0; i < system_size-1; i++) {
          if (randf() < beta) {
            if (randf() < p) {
              measurements.push_back({TWO_QUBIT_PAULI, {i, i+1}});
            } else {
              measurements.push_back({ONE_QUBIT_PAULI, {i}});
            }
          }
        }

        state->measure(measurements);
      } else { 
        std::vector<WeakMeasurementData> measurements;
        for (uint32_t i = 0; i < system_size-1; i++) {
          if (randf() < p) {
            measurements.push_back({TWO_QUBIT_PAULI, {i, i+1}, beta});
          } else {
            measurements.push_back({ONE_QUBIT_PAULI, {i}, beta});
          }
        }

        state->weak_measure(measurements);
      }
    }

    void unitary(uint32_t i, uint32_t j) {
      if (unitary_type == MPSS_HAAR) {
        Eigen::Matrix4cd gate = haar_unitary(2, rng);
        state->evolve(gate, {i, j});
      } else if (unitary_type == MPSS_CLIFFORD) {
        state->random_clifford({i, j});
      } else if (unitary_type == MPSS_Z2_CLIFFORD) {
        z2_table.apply_random(rng, {i, j}, *state.get());
      } else {
        throw std::runtime_error(std::format("Invalid unitary type {}.", unitary_type));
      }
    }

	public:
    std::shared_ptr<MatrixProductState> state;
		MatrixProductSimulator(dataframe::ExperimentParams &params, uint32_t num_threads) : Simulator(params), quantum_sampler(params), z2_table(get_z2_table()) {
      system_size = dataframe::utils::get<int>(params, "system_size");
      beta = dataframe::utils::get<double>(params, "beta");
      p = dataframe::utils::get<double>(params, "p");
      bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 32);

      measurement_type = dataframe::utils::get<int>(params, "measurement_type", MPSS_PROJECTIVE);
      unitary_type = dataframe::utils::get<int>(params, "unitary_type", MPSS_HAAR);

      offset = false;

      state = std::make_shared<MatrixProductState>(system_size, bond_dimension);
      state->seed(rand());

      PauliMutationFunc z2_mutation = [](PauliString& p, std::minstd_rand& rng) {
        size_t n = p.num_qubits;
        PauliString q(n);
        if (rng() % 2) {
          // Single-qubit mutation
          q.set_z(rng() % n, 1);
        } else {
          size_t i = rng() % n;
          size_t j = rng() % n;
          while (j == i) {
            j = rng() % n;
          }

          q.set_x(i, 1);
          q.set_x(j, 1);
        }

        p = p * q;
      };

      quantum_sampler.set_montecarlo_update(z2_mutation);
    }

		virtual void timesteps(uint32_t num_steps) override {
      for (size_t t = 0; t < num_steps; t++) {
        for (size_t i = 0; i < system_size/2 - offset; i++) {
          size_t q1 = 2*i + offset;
          size_t q2 = 2*i + 1 + offset;
          unitary(q1, q2);
        }

        measurement_layer();

        offset = !offset;
      }
    }

    virtual dataframe::data_t take_samples() override {
      dataframe::data_t samples;

      std::vector<double> surface = state->get_entropy_surface<double>(1u);
      dataframe::utils::emplace(samples, "surface", surface);

      quantum_sampler.add_samples(samples, state);

      std::vector<double> bond_dimensions(system_size - 1);
      for (size_t i = 0; i < system_size - 1; i++) {
        bond_dimensions[i] = static_cast<double>(state->bond_dimension(i));
      }
      dataframe::utils::emplace(samples, "bond_dimension_at_site", bond_dimensions);

      dataframe::utils::emplace(samples, "trace", state->trace());

      return samples;
    }
};
