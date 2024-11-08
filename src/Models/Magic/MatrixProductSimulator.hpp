#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

#define MPSS_PROJECTIVE 0
#define MPSS_WEAK 1

#define MPSS_HAAR 0
#define MPSS_CLIFFORD 1
#define MPSS_Z2_CLIFFORD 2

static CliffordTable get_z2_table() {
  auto conserves_xx = [](const QuantumCircuit& qc) {
    PauliString p("XX");
    PauliString p_ = p;
    qc.apply(p);
    return p == p_;
  };

  return CliffordTable(conserves_xx);
}

class MatrixProductSimulator : public dataframe::Simulator {
	private:
		uint32_t system_size;
		double beta;
    double zz_prob;
    size_t bond_dimension;

    int measurement_type;
    int unitary_type;

		InterfaceSampler interface_sampler;
    QuantumStateSampler quantum_sampler;

    CliffordTable z2_table;

    bool offset;

    void measure(uint32_t i, uint32_t j) {
      // TODO check that single-qubit and two-qubit measurements are balanced
      if (measurement_type == MPSS_PROJECTIVE) {
        // Do projective measurement
        if (randf() < beta) {
          if (randf() < zz_prob) {
            state->measure(PauliString("+ZZ"), {i, j});
          } else {
            state->measure(PauliString("+X"), {i});
          }
        }
      } else if (measurement_type == MPSS_WEAK) {
        // Do weak measurement
        if (randf() < zz_prob) {
          state->weak_measure(PauliString("+ZZ"), {i, j}, beta);
        } else {
          state->weak_measure(PauliString("+X"), {i}, beta);
        }
      }

    }

    void measure_right_edge() {
      uint32_t i = system_size - 1;
      if (measurement_type == MPSS_PROJECTIVE) {
        if (randf() < beta && randf() > zz_prob) {
          state->measure(PauliString("+X"), {i});
        }
      } else if (measurement_type == MPSS_WEAK) {
        state->weak_measure(PauliString("+X"), {i}, beta);
      }
    }

    void unitary(uint32_t i, uint32_t j) {
      //std::cout << fmt::format("unitary({}, {})\n", i, j);
      if (unitary_type == MPSS_HAAR) {
        Eigen::Matrix4cd gate = haar_unitary(2, rng);
        state->evolve(gate, {i, j});
      } else if (unitary_type == MPSS_CLIFFORD) {
        state->random_clifford({i, j});
      } else if (unitary_type == MPSS_Z2_CLIFFORD) {
        z2_table.apply_random(rng, {i, j}, *state.get());
      } else {
        throw std::runtime_error(fmt::format("Invalid unitary type {}.", unitary_type));
      }
    }

	public:
    std::shared_ptr<MatrixProductState> state;
		MatrixProductSimulator(dataframe::Params &params, uint32_t num_threads) : Simulator(params), interface_sampler(params), quantum_sampler(params), z2_table(get_z2_table()) {
      system_size = dataframe::utils::get<int>(params, "system_size");
      beta = dataframe::utils::get<double>(params, "beta");
      zz_prob = dataframe::utils::get<double>(params, "zz_prob");
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
        measure_right_edge();

        offset = !offset;
      }
    }

    virtual dataframe::data_t take_samples() override {
      dataframe::data_t samples;

      std::vector<int> surface = state->get_entropy_surface<int>(1u);
      interface_sampler.add_samples(samples, surface);

      quantum_sampler.add_samples(samples, state);

      std::vector<double> bond_dimensions(system_size - 1);
      for (size_t i = 0; i < system_size - 1; i++) {
        bond_dimensions[i] = static_cast<double>(state->bond_dimension(i));
      }
      dataframe::utils::emplace(samples, "bond_dimension_at_site", bond_dimensions);

      return samples;
    }
};
