#pragma once

#include <Simulator.hpp>
#include <Samplers.h>

#define MPSS_PROJECTIVE 0
#define MPSS_WEAK 1
#define MPSS_NONE 2

#define MPSS_HAAR 0
#define MPSS_CLIFFORD 1
#define MPSS_Z2_CLIFFORD 2
#define MPSS_DUAL_CLIFFORD 3

#define MPSS_MPS 0
#define MPSS_STATEVECTOR 1
#define MPSS_CHP 2

#define TWO_QUBIT_PAULI PauliString("+XX")
#define ONE_QUBIT_PAULI PauliString("+Z")

template <typename T>
static CliffordTable<T> get_z2_table() {
  auto symm = [](const QuantumCircuit& qc) {
    PauliString p = PauliString("+ZZ");
    PauliString p_ = p;
    qc.apply(p);
    return p == p_;
  };

  return CliffordTable<T>(symm);
}

static std::vector<QuantumCircuit> get_dual_cliffords() {
  std::vector<QuantumCircuit> circuits;

  QuantumCircuit Z(1);
  Z.sd(0);
  circuits.push_back(Z);
  circuits.push_back(Z.adjoint());

  QuantumCircuit XX(2);
  XX.h(0);
  XX.h(1);
  XX.cx(0, 1);
  XX.sd(1);
  XX.cx(0, 1);
  XX.h(0);
  XX.h(1);
  circuits.push_back(XX);
  circuits.push_back(XX.adjoint());

  QuantumCircuit ZZ(2);
  ZZ.cx(0, 1);
  ZZ.sd(1);
  ZZ.cx(0, 1);
  circuits.push_back(ZZ);
  circuits.push_back(ZZ.adjoint());

  QuantumCircuit XIX(3);
  XIX.append(XX, {0, 2});
  circuits.push_back(XIX);
  circuits.push_back(XIX.adjoint());

  //QuantumCircuit XY(3);
  //XY.s(1);
  //XY.cx(0, 1);
  //XY.h(0);
  //XY.x(0);
  //XY.sd(0);
  //XY.x(0);
  //XY.h(0);
  //XY.cx(0, 1);
  //XY.sd(1);
  //circuits.push_back(XY);
  //circuits.push_back(XY.adjoint());

  //QuantumCircuit YX(3);
  //YX.s(0);
  //YX.cx(1, 0);
  //YX.h(1);
  //YX.x(1);
  //YX.sd(1);
  //YX.x(1);
  //YX.h(1);
  //YX.cx(1, 0);
  //YX.sd(0);
  //circuits.push_back(YX.adjoint());
  //circuits.push_back(YX);

  auto to_matrix = [](const std::string& s) {
    PauliString p(s);
    constexpr std::complex<double> i_pi_over_4 = std::complex<double>(0, 0.78539816339);
    Eigen::MatrixXcd M = (p.to_matrix()*i_pi_over_4).exp()/std::exp(i_pi_over_4);
    return M;
  };

  auto check_equal = [](const Eigen::MatrixXcd& U1, const Eigen::MatrixXcd& U2) {
    for (size_t i = 0; i < U1.rows(); i++) {
      for (size_t j = 0; j < U1.cols(); j++) {
        if (std::abs(U1(i, j) - U2(i, j)) > 1e-5) {
          throw std::runtime_error("Error implementing gate.");
        }
      }
    }
  };

  // Sanity checks
  check_equal(to_matrix("Z"), Z.to_matrix());
  check_equal(to_matrix("XX"), XX.to_matrix());
  check_equal(to_matrix("ZZ"), ZZ.to_matrix());
  check_equal(to_matrix("XIX"), XIX.to_matrix());

  return circuits;
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
    bool sample_entanglement;

    CliffordTable<Eigen::MatrixXcd> z2_table;
    CliffordTable<QuantumCircuit> z2_table_circuits;
    std::vector<QuantumCircuit> dual_circuits;
    std::vector<Eigen::MatrixXcd> dual_gates;

    bool offset;

    void unitary_layer() {
      if (unitary_type == MPSS_HAAR) {
        for (size_t i = 0; i < system_size/2 - offset; i++) {
          size_t q1 = 2*i + offset;
          size_t q2 = 2*i + 1 + offset;
          Eigen::Matrix4cd gate = haar_unitary(2);
          state->evolve(gate, {q1, q2});
        }
      } else if (unitary_type == MPSS_CLIFFORD) {
        for (size_t i = 0; i < system_size/2 - offset; i++) {
          size_t q1 = 2*i + offset;
          size_t q2 = 2*i + 1 + offset;
          state->random_clifford({q1, q2});
        }
      } else if (unitary_type == MPSS_Z2_CLIFFORD) {
        for (size_t i = 0; i < system_size/2 - offset; i++) {
          size_t q1 = 2*i + offset;
          size_t q2 = 2*i + 1 + offset;
          if (state_type == MPSS_CHP) {
            z2_table_circuits.apply_random({q1, q2}, *state.get());
          } else {
            z2_table.apply_random({q1, q2}, *state.get());
          }
        }
      } else if (unitary_type == MPSS_DUAL_CLIFFORD) {
        auto get_qubits = [](const QuantumCircuit& circuit, uint32_t q) {
          uint32_t nqb = circuit.get_num_qubits();
          if (nqb == 1) {
            return Qubits{q};
          } else if (nqb == 2) {
            return Qubits{q, q+1};
          } else {
            return Qubits{q, q+1, q+2};
          }
        };

        for (size_t i = 0; i < system_size; i++) {
          // Select random qubit and random qubits
          uint32_t q = randi(0, system_size - 2);
          size_t r = randi(0, dual_circuits.size());
          Qubits qubits = get_qubits(dual_circuits[r], q);
          if (state_type == MPSS_CHP) {
            const QuantumCircuit& circuit = dual_circuits[r];
            state->evolve(circuit, qubits);
          } else {
            const Eigen::MatrixXcd& gate = dual_gates[r];
            state->evolve(gate, qubits);
          }
        }
      } else {
        throw std::runtime_error(fmt::format("Invalid unitary type {}.", unitary_type));
      }
    }

    void measurement_layer() {
      if (measurement_type == MPSS_PROJECTIVE) {
        for (uint32_t i = 0; i < system_size-1; i++) {
          size_t q = randi(0, system_size-1);
          if (randf() < beta) {
            if (randf() < p) {
              state->measure(Measurement({q, q+1}, TWO_QUBIT_PAULI));
            } else {
              state->measure(Measurement({q}, ONE_QUBIT_PAULI));
            }
          }
        }
      } else if (measurement_type == MPSS_WEAK) { 
        for (uint32_t i = 0; i < system_size; i++) {
          size_t q = randi(0, system_size-1);
          if (randf() < p) {
            state->weak_measure(WeakMeasurement({q, q+1}, beta, TWO_QUBIT_PAULI));
          } else {
            state->weak_measure(WeakMeasurement({q}, beta, ONE_QUBIT_PAULI));
          }
        }
      } else if (measurement_type == MPSS_NONE) {
        return;
      }
    }

	public:
    std::shared_ptr<QuantumState> state;

		MatrixProductSimulator(dataframe::ExperimentParams &params, uint32_t num_threads) : Simulator(params), quantum_sampler(params), z2_table(get_z2_table<Eigen::MatrixXcd>()), dual_circuits(get_dual_cliffords()) {
      system_size = dataframe::utils::get<int>(params, "system_size");
      beta = dataframe::utils::get<double>(params, "beta");
      p = dataframe::utils::get<double>(params, "p");

      measurement_type = dataframe::utils::get<int>(params, "measurement_type", MPSS_PROJECTIVE);
      unitary_type = dataframe::utils::get<int>(params, "unitary_type", MPSS_HAAR);

      state_type = dataframe::utils::get<int>(params, "state_type", MPSS_MPS);
      sample_entanglement = dataframe::utils::get<int>(params, "sample_entanglement", true);

      for (size_t i = 0; i < dual_circuits.size(); i++) {
        dual_gates.push_back(dual_circuits[i].to_matrix());
      }

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
      } else if (state_type == MPSS_CHP) {
        participation_sampler = std::make_unique<CHPParticipationSampler>(params);
        z2_table_circuits = get_z2_table<QuantumCircuit>();
        state = std::make_shared<QuantumCHPState>(system_size);
      }

      offset = false;
    }

		virtual void timesteps(uint32_t num_steps) override {
      for (size_t t = 0; t < num_steps; t++) {
        unitary_layer();
        measurement_layer();
        offset = !offset;
      }
    }

    virtual dataframe::SampleMap take_samples() override {
      dataframe::SampleMap samples;

      participation_sampler->add_samples(samples, state);
      quantum_sampler.add_samples(samples, state);

      if (state_type != MPSS_CHP) {
        magic_sampler->add_samples(samples, std::dynamic_pointer_cast<MagicQuantumState>(state));
      }

      if (sample_entanglement) {
        std::vector<double> entanglement = state->get_entanglement<double>(1u);
        dataframe::utils::emplace(samples, "entanglement", entanglement);
      }

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
