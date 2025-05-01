#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

#define MPSS_PROJECTIVE 0
#define MPSS_WEAK 1
#define MPSS_NONE 2

#define MPSS_HAAR 0
#define MPSS_CLIFFORD 1
#define MPSS_Z2_CLIFFORD 2

#define MPSS_MPS 0
#define MPSS_STATEVECTOR 1

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
    int state_type;

    std::unique_ptr<ParticipationSampler> participation_sampler;
    std::unique_ptr<StabilizerEntropySampler> magic_sampler;
    QuantumStateSampler quantum_sampler;

    CliffordTable z2_table;

    bool offset;

    void measurement_layer() {
      if (measurement_type == MPSS_PROJECTIVE) {
        for (uint32_t i = 0; i < system_size-1; i++) {
          if (randf() < beta) {
            if (randf() < p) {
              state->measure(Measurement({i, i+1}, TWO_QUBIT_PAULI));
            } else {
              state->measure(Measurement({i}, ONE_QUBIT_PAULI));
            }
          }
        }

        // --------------- //
        if (randf() >= p) {
          state->measure(Measurement({system_size-1}, ONE_QUBIT_PAULI));
        }
        // --------------- //
      } else if (measurement_type == MPSS_WEAK) { 
        for (uint32_t i = 0; i < system_size-1; i++) {
          if (randf() < p) {
            state->weak_measure(WeakMeasurement({i, i+1}, beta, TWO_QUBIT_PAULI));
          } else {
            state->weak_measure(WeakMeasurement({i}, beta, ONE_QUBIT_PAULI));
          }
        }

        // --------------- //
        if (randf() >= p) {
          state->weak_measure(WeakMeasurement({system_size-1}, beta, ONE_QUBIT_PAULI));
        }
        // --------------- //
      } else if (measurement_type == MPSS_NONE) {
        return;
      }
    }

    void unitary(uint32_t i, uint32_t j) {
      if (unitary_type == MPSS_HAAR) {
        Eigen::Matrix4cd gate = haar_unitary(2);
        state->evolve(gate, {i, j});
      } else if (unitary_type == MPSS_CLIFFORD) {
        state->random_clifford({i, j});
      } else if (unitary_type == MPSS_Z2_CLIFFORD) {
        z2_table.apply_random({i, j}, *state.get());
      } else {
        throw std::runtime_error(std::format("Invalid unitary type {}.", unitary_type));
      }
    }

	public:
    std::shared_ptr<MagicQuantumState> state;
		MatrixProductSimulator(dataframe::ExperimentParams &params, uint32_t num_threads) : Simulator(params), quantum_sampler(params), z2_table(get_z2_table()) {
      system_size = dataframe::utils::get<int>(params, "system_size");
      beta = dataframe::utils::get<double>(params, "beta");
      p = dataframe::utils::get<double>(params, "p");

      measurement_type = dataframe::utils::get<int>(params, "measurement_type", MPSS_PROJECTIVE);
      unitary_type = dataframe::utils::get<int>(params, "unitary_type", MPSS_HAAR);

      state_type = dataframe::utils::get<int>(params, "state_type", MPSS_MPS);
      if (state_type == MPSS_MPS) {
        participation_sampler = std::make_unique<MPSParticipationSampler>(params);
        magic_sampler = std::make_unique<MPSMagicSampler>(params);

        bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 32);

        int mps_debug_level = dataframe::utils::get<int>(params, "mps_debug_level", 0);
        state = std::make_shared<MatrixProductState>(system_size, bond_dimension);

        MatrixProductState* mps = dynamic_cast<MatrixProductState*>(state.get());
        mps->set_debug_level(mps_debug_level);
      } else if (state_type == MPSS_STATEVECTOR) {
        PauliMutationFunc z2_mutation = [](PauliString& p) {
          size_t n = p.num_qubits;
          PauliString q(n);
          if (randi() % 2) {
            // Single-qubit mutation
            q.set_z(randi() % n, 1);
          } else {
            size_t i = randi() % n;
            size_t j = randi() % n;
            while (j == i) {
              j = randi() % n;
            }

            q.set_x(i, 1);
            q.set_x(j, 1);
          }

          p = p * q;
        };

        participation_sampler = std::make_unique<GenericParticipationSampler>(params);
        magic_sampler = std::make_unique<GenericMagicSampler>(params);

        dynamic_cast<GenericMagicSampler*>(magic_sampler.get())->set_montecarlo_update(z2_mutation);

        state = std::make_shared<Statevector>(system_size);
      }

      offset = false;

      std::string filename = dataframe::utils::get<std::string>(params, "filename", "");
      int s;
      if (filename != "") {
        s = load_seed(filename);
        std::cout << fmt::format("Found filename = {}, loaded seed = {}\n", filename, s);
      } else {
        s = randi();
      }
      Random::seed_rng(s);
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

    virtual dataframe::SampleMap take_samples() override {
      dataframe::SampleMap samples;

      participation_sampler->add_samples(samples, state);
      magic_sampler->add_samples(samples, state);
      quantum_sampler.add_samples(samples, state);

      std::vector<double> entanglement = state->get_entropy_surface<double>(1u);
      dataframe::utils::emplace(samples, "entanglement", entanglement);

      if (state_type == MPSS_MPS) {
        std::vector<double> bond_dimensions(system_size - 1);
        MatrixProductState* mps = dynamic_cast<MatrixProductState*>(state.get());
        for (size_t i = 0; i < system_size - 1; i++) {
          bond_dimensions[i] = static_cast<double>(mps->bond_dimension(i));
        }
        dataframe::utils::emplace(samples, "bond_dimension_at_site", bond_dimensions);

        std::vector<double> truncerr = mps->get_logged_truncerr();
        double p = 0.0;
        for (size_t i = 0; i < truncerr.size(); i++) {
          p += truncerr[i];
        }

        dataframe::utils::emplace(samples, "truncerr", p/truncerr.size());
      }

      return samples;
    }

    virtual std::vector<char> serialize() const override {
      return state->serialize();
    }

    virtual void deserialize(const std::vector<char>& bytes) override {
      state = std::make_shared<MatrixProductState>(system_size, bond_dimension);
      state->deserialize(bytes);
    }
};
