#pragma once

#include <Frame.h>

#include <Samplers.h>
#include <QuantumState.h>

#include <unsupported/Eigen/KroneckerProduct>

#define QIT_MPS 0
#define QIT_STATEVECTOR 1

static Eigen::MatrixXd make_gate(const Eigen::Matrix2d& g, size_t q, size_t num_qubits) {
  if (num_qubits > 15) {
    throw std::runtime_error("Cannot make_gate for num_qubits > 15.");
  }

  size_t s1 = 1u << q;
  Eigen::MatrixXd I1 = Eigen::MatrixXd::Identity(s1, s1);
  size_t s2 = 1u << (num_qubits - q - 1);
  Eigen::MatrixXd I2 = Eigen::MatrixXd::Identity(s2, s2);


  Eigen::MatrixXd G1 = Eigen::kroneckerProduct(I1, g);
  Eigen::MatrixXd G2 = Eigen::kroneckerProduct(G1, I2);
  return G2;
}

static Eigen::MatrixXd make_gate(const Eigen::Matrix2d& g1, const Eigen::Matrix2d& g2, size_t q1, size_t q2, size_t num_qubits) {
  return make_gate(g1, q1, num_qubits) * make_gate(g2, q2, num_qubits);
}

Statevector quantum_ising_ground_state(size_t num_qubits, double h) {
  size_t s = 1u << num_qubits;
  Eigen::MatrixXd H = Eigen::MatrixXd::Zero(s, s);

  Eigen::Matrix2d Z; Z << 1.0, 0.0, 0.0, -1.0;
  Eigen::Matrix2d X; X << 0.0, 1.0, 1.0, 0.0;

  // Construct Hamiltonian
  for (size_t i = 0; i < num_qubits-1; i++) {
    H -= make_gate(X, X, i, i+1, num_qubits);
  }

  for (size_t i = 0; i < num_qubits; i++) {
    H -= h*make_gate(Z, i, num_qubits);
  }

  // Compute ground state of H
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(H);
  Eigen::VectorXcd vec = solver.eigenvectors().col(0);

  Statevector sv(vec);
  return sv;
}


class QuantumIsingTestConfig {
  public:
    size_t system_size;
    size_t bond_dimension;
    size_t num_sweeps;

    int state_type;
    double h;
    double delta;

    QuantumStateSampler quantum_sampler;
    EntropySampler entropy_sampler;

    QuantumIsingTestConfig(dataframe::Params &params) : quantum_sampler(params), entropy_sampler(params) {
      system_size = dataframe::utils::get<int>(params, "system_size", 1);
      bond_dimension = dataframe::utils::get<int>(params, "bond_dimension", 64);
      h = dataframe::utils::get<double>(params, "h");

      num_sweeps = dataframe::utils::get<int>(params, "num_sweeps", 10);

      state_type = dataframe::utils::get(params, "state_type", QIT_MPS);

      auto xxz_mutation = [](PauliString& p, std::minstd_rand& rng) {
        PauliString pnew(p);
        if ((rng() % 2) || (p.num_qubits == 1)) {
          // Do single-qubit mutation
          size_t j = rng() % p.num_qubits;
          PauliString Zj = PauliString(p.num_qubits);
          Zj.set_z(j, 1); 

          pnew *= Zj;
        } else {
          // Do double-qubit mutation
          size_t j1 = rng() % p.num_qubits;
          size_t j2 = rng() % p.num_qubits;
          while (j2 == j1) {
            j2 = rng() % p.num_qubits;
          }

          PauliString Xij = PauliString(p.num_qubits);
          Xij.set_x(j1, 1); 
          Xij.set_x(j2, 1); 
          pnew *= Xij;
        }

        p = pnew;
      };

      quantum_sampler.set_montecarlo_update(xxz_mutation);
    }

    dataframe::DataSlide compute(uint32_t num_threads) {
      auto start = std::chrono::high_resolution_clock::now();

      std::shared_ptr<QuantumState> state;
      if (state_type == QIT_MPS) {
        state = std::make_shared<MatrixProductState>(MatrixProductState::ising_ground_state(system_size, h, bond_dimension, 1e-8, num_sweeps));
      } else if (state_type == QIT_STATEVECTOR) {
        state = std::make_shared<Statevector>(quantum_ising_ground_state(system_size, h));
      }

      dataframe::DataSlide slide;
      dataframe::data_t samples;

      auto surface = state->get_entropy_surface<double>(1);

      slide.add_data("surface", surface.size());
      slide.push_samples_to_data("surface", surface);

      quantum_sampler.add_samples(samples, state);
      entropy_sampler.add_samples(samples, state);
      slide.add_samples(samples);
      slide.push_samples(samples);

      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
      slide.add_data("time");
      slide.push_samples_to_data("time", duration.count());

      return slide;
    }
};
