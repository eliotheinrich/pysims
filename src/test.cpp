#include <iostream>
#include "QuantumState.h"
#include "GroverProjectionSimulator.h"
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <unsupported/Eigen/MatrixFunctions>
#include <vector>
#include <map>
#include <Frame.h>
#include <Samplers.h>

using namespace dataframe;
using namespace dataframe::utils;

std::string print_pair(std::pair<double, double> p) {
  return std::to_string(p.first) + ", " + std::to_string(p.second);
}

bool test_Statevector() {
  Eigen::MatrixXcd X(2, 2);
  X << 0, 1, 1, 0;

  Eigen::MatrixXcd Y(2, 2);
  Y << 0, -1, 1, 0;
  Y *= std::complex<double>(0, 1);

  Eigen::MatrixXcd Z(2, 2);
  Z << 1, 0, 0, -1;

  Eigen::MatrixXcd H(2, 2);
  H << 1, 1, 1, -1;
  H /= std::sqrt(2);

  Eigen::MatrixXcd CX(4, 4);
  CX << 1, 0, 0, 0,
        0, 0, 0, 1,
        0, 0, 1, 0,
        0, 1, 0, 0;

  //state.evolve(H, std::vector<uint32_t>{0});
  std::mt19937 rng;
  rng.seed(10);
  auto gate = haar_unitary(2, rng);
  std::cout << std::fixed << std::setprecision(12) << gate << std::endl << std::endl;
  Statevector state(3);
  state.evolve(gate, std::vector<uint32_t>{0, 2});
  std::cout << state.to_string() << "\n\n";
  //std::cout << state.entropy(std::vector<uint32_t>{0}, 2) << std::endl;
  //std::cout << state.entropy(std::vector<uint32_t>{1}, 2) << std::endl;
  //std::cout << state.entropy(std::vector<uint32_t>{2}, 2) << std::endl;
  //std::cout << print_pair(state.probabilities(0)) << std::endl;
  //std::cout << print_pair(state.probabilities(1)) << std::endl;
  //std::cout << print_pair(state.probabilities(2)) << std::endl;

  //state.evolve(CX, std::vector<uint32_t>{0, 1});
  //std::cout << state.to_string() << "\n\n";
  //std::cout << state.entropy(std::vector<uint32_t>{0}, 2) << std::endl;
  //std::cout << state.entropy(std::vector<uint32_t>{1}, 2) << std::endl;
  //std::cout << state.entropy(std::vector<uint32_t>{2}, 2) << std::endl;

  //std::cout << print_pair(state.probabilities(0)) << std::endl;
  //std::cout << print_pair(state.probabilities(1)) << std::endl;
  //std::cout << print_pair(state.probabilities(2)) << std::endl;

  return true;
}

bool test_GroverSimulation() {
  std::cout << "Beginning grover simulation\n";
  Params p;
  p["system_size"] = 3.0;
  p["mzr_prob"] = (double) 0.0;
  p["nmax"] = 50.0;

  GroverProjectionSimulator gs(p,1);

  //for (uint32_t i = 0; i < 10; i++) {
  //    auto gate = haar_unitary(3);
  //    gs.state->evolve(haar_unitary(3));

  //    uint32_t q = gs.rand() % 3;
  //    gs.grover_projection(q, 1);

  //    auto probs = gs.state->probabilities(q);
  //    std::cout << gs.state->get_statevector().data << std::endl;
  //    std::cout << "on qubit " << q << ": " << print_pair(probs) << " sum to " << probs.first + probs.second << std::endl;
  //    std::cout << "entropy of projected qubit: " << gs.entropy(std::vector<uint32_t>{q}, 2) << "\n\n";
  //}

  std::cout << gs.create_oracle(3, std::vector<uint32_t>{1}) << std::endl;
  std::cout << gs.create_oracle(3, std::vector<uint32_t>{0,1}) << std::endl;
  std::cout << gs.create_oracle(3, std::vector<uint32_t>{0,1,2}) << std::endl;

  return true;
}

bool test_UnitaryEquivalence() {
  uint32_t num_qubits = 3;
  auto gate = haar_unitary(num_qubits);

  Statevector state_vector(num_qubits);
  UnitaryState state_unitary(num_qubits);

  state_vector.evolve(gate);
  state_unitary.evolve(gate);

  std::cout << state_vector.to_string() << std::endl;
  std::cout << state_unitary.to_string() << std::endl;

  return true;
}

bool test_normalize() {
  uint32_t num_qubits = 3;

  Statevector state(num_qubits);
  state.data = state.data/2;
  state.evolve(haar_unitary(num_qubits));

  std::cout << state.to_string() << std::endl;
  std::cout << state.norm() << std::endl;

  state.normalize();

  std::cout << state.to_string() << std::endl;
  std::cout << state.norm() << std::endl;


  std::cout << "\nBeginning UnitaryState tests\n\n";
  auto gate = haar_unitary(num_qubits);
  gate(0,0) += 1e-8;

  UnitaryState ustate1(num_qubits);
  ustate1.evolve(gate);

  for (auto e : ustate1.unitary.eigenvalues())
    std::cout << std::abs(e) << " ";
  std::cout << "\n\n";

  UnitaryState ustate2(num_qubits);
  ustate2.evolve(gate.pow(10000000));
  for (auto e : ustate2.unitary.eigenvalues())
    std::cout << std::abs(e) << " ";
  std::cout << "\n";
  std::cout << ustate2.get_statevector().to_string();
  std::cout << "\n\n";

  ustate2.normalize();
  for (auto e : ustate2.unitary.eigenvalues())
    std::cout << std::abs(e) << " ";
  std::cout << "\n";
  std::cout << ustate2.get_statevector().to_string();
  std::cout << "\n\n";

  return true;
}

bool test_density_matrix() {
  uint32_t num_qubits = 2;
  DensityMatrix d(num_qubits);
  std::cout << d.data << std::endl << std::endl;

  QuantumCircuit qc = generate_haar_circuit(num_qubits, 1);
  d.QuantumState::evolve(qc);

  std::cout << d.data << std::endl << std::endl;


  std::cout << d.data << std::endl << std::endl;

  return true;
}

#include <VQSE.hpp>
bool test_vqse() {
  uint32_t num_qubits = 4;
  std::cout << "Starting\n";
  QuantumCircuit target_circuit = generate_haar_circuit(num_qubits, 2);
  target_circuit.add_measurement(std::vector<uint32_t>{1});

  std::vector<std::string> rotation_gates{"rx", "ry", "rz"};
  QuantumCircuit ansatz = hardware_efficient_ansatz(num_qubits, 2, rotation_gates);
  std::vector<double> initial_params(ansatz.num_params());
  uint32_t m = 4;
  uint32_t max_iter = 1;

  VQSE vqse(ansatz, m, max_iter);
  std::cout << "vqse built\n";

  // vqse has taken ownership of ansatz; re-generate new copy.
  //ansatz = hardware_efficient_ansatz(num_qubits, 2);

  //vqse.optimize(target_circuit, initial_params);

  //for (auto e : vqse.true_eigenvalues(target_circuit)) std::cout << e << " ";
  //std::cout << "\n";

  //for (auto e : vqse.eigenvalue_estimates) std::cout << e << " ";
  //std::cout << "\n";
  for (uint32_t i = 0; i < num_qubits; i++) {
    std::cout << "r[" << i << "] = " << vqse.r[i] << std::endl;
    std::cout << "q[" << i << "] = " << vqse.q[i] << std::endl;
  }


  for (uint32_t i = 0; i < (1u << num_qubits); i++) {
    std::cout << i << ":" << std::endl;
    std::cout << "global: " << vqse.global_energy_by_bitstring(i) << std::endl;
    std::cout << "local: " << vqse.local_energy_by_bitstring(i) << std::endl;
  }


  return true;
}

bool test_vqse_energy() {
  uint32_t num_qubits = 8;
  for (uint32_t m = 1; m < 2; m++) {
    QuantumCircuit target_circuit = generate_haar_circuit(num_qubits, 2);
    target_circuit.add_measurement(std::vector<uint32_t>{1});
    DensityMatrix target(target_circuit);

    std::vector<std::string> rotation_gates{"rx", "ry", "rz"};
    QuantumCircuit ansatz = hardware_efficient_ansatz(num_qubits, 2, rotation_gates);
    std::vector<double> initial_params(ansatz.num_params());
    uint32_t max_iter = 500;

    VQSE vqse(std::move(ansatz), m, max_iter);

    auto local_energy_levels = vqse.get_local_energy_levels();
    std::vector<std::string> local_energy_levels_str;
    uint32_t s = m + 5;
    for (uint32_t i = 0; i < s; i++)
      local_energy_levels_str.push_back(std::to_string(local_energy_levels[i]));
    auto global_energy_levels = vqse.get_global_energy_levels();
    std::vector<std::string> global_energy_levels_str;
    for (uint32_t i = 0; i < s; i++)
      global_energy_levels_str.push_back(std::to_string(global_energy_levels[i]));

    std::cout << "m = " << m << std::endl;
    std::cout << "Local energy: " << join(local_energy_levels_str, " ") << std::endl;
    std::cout << "Global energy: " << join(global_energy_levels_str, " ") << std::endl;
  }
  return true;
}

#include <random>
#include "Models.h"
#include <sstream>

void print_params(std::vector<double> params) {
  std::cout << "[ ";
  for (auto p : params) std::cout << p << " ";
  std::cout << "]\n";
}

double grad_fd(QuantumCircuit qc, std::vector<double> params, Eigen::MatrixXcd obs, double epsilon=1e-3) {
  auto params_plus = params;
  params_plus[0] += epsilon;
  auto params_minus = params;
  params_minus[0] -= epsilon;

  print_params(params_plus);
  print_params(params_minus);

  double cost_plus = (DensityMatrix(qc.bind_params(params_plus)).data * obs).trace().real();
  double cost_minus = (DensityMatrix(qc.bind_params(params_minus)).data * obs).trace().real();

  return (cost_plus - cost_minus)/(2.0*epsilon);
}

double grad_ps(QuantumCircuit qc, std::vector<double> params, Eigen::MatrixXcd obs) {
  auto params_plus = params;
  params_plus[0] += PI/2;
  auto params_minus = params;
  params_minus[0] -= PI/2;

  print_params(params_plus);
  print_params(params_minus);

  double cost_plus = (DensityMatrix(qc.bind_params(params_plus)).data * obs).trace().real();
  double cost_minus = (DensityMatrix(qc.bind_params(params_minus)).data * obs).trace().real();

  return (cost_plus - cost_minus)/2.0;
}


bool test_parametrized_circuit() {
  std::vector<double> p{1.0};

  QuantumCircuit qc(1);
  qc.add_gate(parse_gate("Rx", std::vector<uint32_t>{0}));

  std::cout << qc.to_string() << std::endl;
  std::cout << "Gate op: \n";
  std::cout << qc.bind_params(p).to_matrix() << std::endl;

  Eigen::Matrix2cd Z; Z << 1.0, 0.0, 0.0, -1.0;

  std::cout << "grad_fd: " << grad_fd(qc, p, Z) << std::endl;
  std::cout << "grad_ps: " << grad_ps(qc, p, Z) << std::endl;

  return true;
}

#include <fstream>
#include <iomanip>

void small_chp_test() {
  uint32_t system_size = 2;
  int seed = 314;
  QuantumCHPState state1(system_size, seed);
  QuantumGraphState state2(system_size, seed);

  std::vector<uint32_t> qbits{0, 1};

  //auto state3 = state2.to_chp();

  state1.random_clifford(qbits);
  state2.random_clifford(qbits);

  //state1.h(1);
  //state1.cx(0, 1);
  //state1.cx(1, 0);
  //state1.cx(0, 1);
  //state1.s(0);
  //state1.s(0);
  //state1.h(0);
  //state1.s(0);
  //state1.s(0);
  //state1.h(0);
  //state1.h(1);
  //state1.cx(0, 1);
  //state1.cx(1, 0);
  //state1.cx(0, 1);
  //state1.s(0);
  //state1.s(0);
  //state1.h(0);
  //state1.s(0);
  //state1.s(0);
  //state1.h(0);
  //state1.h(0);
  //state1.cx(0, 1);
  //state1.h(0);

  //state1.h(1);
  //state1.s(1);
  //state1.y_gate(1);

  //state2.h(1);
  //state2.cx(0, 1);
  //state2.cx(1, 0);
  //state2.cx(0, 1);
  //state2.s(0);
  //state2.s(0);
  //state2.h(0);
  //state2.s(0);
  //state2.s(0);
  //state2.h(0);
  //state2.h(1);
  //state2.cx(0, 1);
  //state2.cx(1, 0);
  //state2.cx(0, 1);
  //state2.s(0);
  //state2.s(0);
  //state2.h(0);
  //state2.s(0);
  //state2.s(0);
  //state2.h(0);
  //state2.h(0);
  //state2.cx(0, 1);
  //state2.h(0);
  //state2.h(1);
  //state2.s(1);
  //state2.y_gate(1);



  //state1.h(0);
  //state1.cx(0, 1);

  //state2.h(0);
  //state2.cx(0, 1);


  auto state3 = state2.to_chp();
  std::cout << "state1 == state1: " << (state1 == state1) << std::endl;
  std::cout << "state1 == state2.to_chp(): " << (state1 == state3) << std::endl;

  if (state1 != state3) {
    std::cout << "state1: \n" << state1.to_string_ops() << std::endl;
    std::cout << "state3: \n" << state3.to_string_ops() << std::endl;
    std::cout << "state2: \n" << state2.to_string() << std::endl;
    throw std::invalid_argument("States do not match.");
  }
}

struct sgate {
  int q;
};

struct hgate {
  int q;
};

struct cxgate {
  int q1;
  int q2;
};

using gate = std::variant<sgate, hgate, cxgate>;

struct state_evolver {
  std::shared_ptr<CliffordState> state;

  state_evolver(std::shared_ptr<CliffordState> state) : state(state) {}

  void operator()(sgate g) {
    state->s(g.q);
  }

  void operator()(hgate g) {
    state->h(g.q);
  }

  void operator()(cxgate g) {
    state->cx(g.q1, g.q2);
  }
};

struct sv_evolver {
  std::shared_ptr<Statevector> state;
  uint32_t num_qubits;

  sv_evolver(std::shared_ptr<Statevector> state) : state(state) {
    num_qubits = state->num_qubits;
  }

  void operator()(sgate g) {
    Eigen::Matrix2cd S; S << std::complex<double>(1, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 1);
    state->QuantumState::evolve(S, g.q);
  }

  void operator()(hgate g) {
    Eigen::Matrix2cd H; H << std::complex<double>(1, 0), std::complex<double>(1, 0), std::complex<double>(1, 0), std::complex<double>(-1, 0); H /= std::sqrt(2);
    state->QuantumState::evolve(H, g.q);
  }

  void operator()(cxgate g) {
    Eigen::Matrix4cd CX; CX << std::complex<double>(1, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), 
      std::complex<double>(0, 0), std::complex<double>(1, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), 
      std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0), 
      std::complex<double>(0, 0), std::complex<double>(0, 0), std::complex<double>(1, 0), std::complex<double>(0, 0);
    std::vector<uint32_t> qubits{static_cast<uint32_t>(g.q2), static_cast<uint32_t>(g.q1)};
    state->evolve(CX, qubits);
  }
};

struct execution_printer {
  std::string operator()(sgate g) {
    return "S on " + std::to_string(g.q);
  }

  std::string operator()(hgate g) {
    return "H on " + std::to_string(g.q);
  }

  std::string operator()(cxgate g) {
    return "CX on " + std::to_string(g.q1) + " " + std::to_string(g.q2);
  }
};

void circuit_test() {
  uint32_t system_size = 2;

  std::vector<gate> circuit;
  circuit.push_back(sgate{0}); // 0
  circuit.push_back(cxgate{0,1}); // 1
  circuit.push_back(sgate{1}); // 4
  circuit.push_back(cxgate{1,0}); // 1
  circuit.push_back(hgate{1}); // 5
  circuit.push_back(cxgate{1,0}); // 1

  circuit.push_back(sgate{1}); // 7
  circuit.push_back(sgate{1}); // 8
  circuit.push_back(hgate{1}); // 9

  circuit.push_back(sgate{1}); // 7
  circuit.push_back(sgate{1}); // 8
  circuit.push_back(hgate{1}); // 9
  circuit.push_back(hgate{1}); // 10


  circuit.push_back(cxgate{0,1}); // 11
  circuit.push_back(cxgate{1,0}); // 12
  circuit.push_back(cxgate{0,1}); // 13
  circuit.push_back(sgate{0}); // 14
  circuit.push_back(sgate{0}); // 15
  circuit.push_back(hgate{0}); // 16
  circuit.push_back(sgate{0}); // 17
  circuit.push_back(sgate{0}); // 18
  circuit.push_back(cxgate{0,1});
  circuit.push_back(hgate{0});
  circuit.push_back(hgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(hgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(hgate{1});

  circuit.push_back(hgate{1});
  circuit.push_back(cxgate{0,1});
  circuit.push_back(hgate{0});
  circuit.push_back(cxgate{1,0});
  circuit.push_back(hgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(hgate{1});
  circuit.push_back(sgate{1});
  circuit.push_back(sgate{1});

  circuit.push_back(hgate{0});
  circuit.push_back(sgate{0});
  circuit.push_back(sgate{0});

  std::shared_ptr<QuantumCHPState> state1 = std::make_shared<QuantumCHPState>(system_size);
  std::shared_ptr<QuantumGraphState> state2 = std::make_shared<QuantumGraphState>(system_size);
  state2->graph.set_val(0, 4);
  state2->graph.set_val(1, 2);
  std::shared_ptr<Statevector> state3 = std::make_shared<Statevector>(system_size);
  state_evolver se1(state1);
  state_evolver se2(state2);
  sv_evolver se3(state3);

  uint32_t i = 0;
  for (auto const g : circuit) {
    std::cout << "\n\n\n" << std::visit(execution_printer(), g) << std::endl;
    auto state2_chp = state2->to_chp();

    std::cout << "Before: \n";
    std::cout << "state1: " << state1->to_string_ops() << std::endl;
    std::cout << "state2_chp: " << state2_chp.to_string_ops() << std::endl;
    std::cout << "state2: " << state2->to_string() << std::endl;

    auto sv1 = state1->to_statevector();
    auto sv2 = state2->to_statevector();
    std::cout << "state1_sv: " << sv1.to_string() << std::endl;
    std::cout << "state2_sv: " << sv2.to_string() << std::endl;
    std::cout << "state3   : " << state3->to_string() << std::endl;

    std::visit(se1, g);
    std::visit(se2, g);
    std::visit(se3, g);


    state2_chp = state2->to_chp();

    std::cout << "\nAfter: \n";
    std::cout << "state1: " << state1->to_string_ops() << std::endl;
    std::cout << "state2_chp: " << state2_chp.to_string_ops() << std::endl;
    std::cout << "state2: " << state2->to_string() << std::endl;

    sv1 = state1->to_statevector();
    sv2 = state2->to_statevector();
    std::cout << "state1_sv: " << sv1.to_string() << std::endl;
    std::cout << "state2_sv: " << sv2.to_string() << std::endl;
    std::cout << "state3   : " << state3->to_string() << std::endl;

    state1->tableau.rref();
    state2_chp.tableau.rref();
    if (*state1 != state2_chp) {
      std::cout << "On step " << i << std::endl;
      throw std::invalid_argument("States not equal.");
    }
    i++;
  }




}



void large_chp_test_multiqubit() {
  uint32_t system_size = 33;
  int seed = 314;
  QuantumCHPState state1(system_size, seed);
  QuantumGraphState state2(system_size, seed);

  std::mt19937 rng(seed);

  uint32_t num_iters = 1000;
  std::cout << "Initial state: \n" << state1.to_string() << std::endl << state1.to_string_ops() << std::endl;

  for (uint32_t i = 0; i < num_iters; i++) {
    uint32_t q1 = rng() % system_size;
    uint32_t q2 = rng() % system_size;
    while (q2 == q1) {
      q2 = rng() % system_size;
    }

    std::vector<uint32_t> qbits{q1, q2};
    state1.random_clifford(qbits);
    state2.random_clifford(qbits);

    uint32_t q3 = rng() % system_size;

    if (double(rng()) / RAND_MAX < 0.05) {
      //state1.mzr(q3);
      //state2.mzr(q3);
    }
    auto state3 = state2.to_chp();

    state1.tableau.rref();
    state3.tableau.rref();


    //std::cout << state1.to_string() << std::endl;
    //std::cout << state3.to_string() << std::endl;

    if (state1 != state3) {
      Statevector sv1 = state1.to_statevector();
      Statevector sv2 = state2.to_statevector();
      std::cout << "state1: \n" << state1.to_string() << std::endl;
      std::cout << "state2: \n" << state2.to_string() << std::endl;
    }


    std::cout << "CHP state:   [ ";
    std::vector<int> s1 = state1.get_entropy_surface<int>();
    for (uint32_t j = 0; j < system_size; j++) {
      std::cout << s1[j] << " ";
    } std::cout << "]\n";

    std::cout << "Graph state: [ ";
    std::vector<int> s2 = state2.get_entropy_surface<int>();
    for (uint32_t j = 0; j < system_size; j++) {
      std::cout << s2[j] << " ";
    } std::cout << "]\n\n";


    for (uint32_t j = 0; j < system_size; j++) {
      if (s1[j] != s2[j]) {
        std::vector<uint32_t> qubits(j+1);
        std::iota(qubits.begin(), qubits.end(), 0);

        state1.tableau.rref();
        std::cout << state1.to_string() << std::endl;
        state1.entropy(qubits, 1);

        std::string error_message = "Surfaces do not agree at step " + std::to_string(i) + ".";
        throw std::invalid_argument(error_message);
      }
    }
  }
}

void large_chp_test_singlequbit() {
  uint32_t system_size = 33;
  int seed = 314;
  QuantumCHPState state1(system_size, seed);
  QuantumGraphState state2(system_size, seed);

  std::mt19937 rng(seed);

  uint32_t num_iters = 1000;

  for (uint32_t i = 0; i < num_iters; i++) {
    uint32_t q1 = rng() % system_size;

    std::vector<uint32_t> qbits{q1};
    std::cout << "Random clifford on " << q1 << std::endl;
    state1.random_clifford(qbits);
    state2.random_clifford(qbits);

    auto state3 = state2.to_chp();
    state1.tableau.rref();
    state3.tableau.rref();

    state3 = state2.to_chp();

    std::cout << "state1 == state1: " << (state1 == state1) << std::endl;
    std::cout << "state1 == state2.to_chp(): " << (state1 == state3) << std::endl;

    if (state1 != state3) {
      std::cout << "state1: \n" << state1.to_string() << std::endl;
      std::cout << "state3: \n" << state3.to_string() << std::endl;
      std::cout << "state2: \n" << state2.to_string() << std::endl;
      Statevector sv1 = state1.to_statevector();
      Statevector sv2 = state2.to_statevector();
      Statevector sv3 = state3.to_statevector();
      //Statevector sv4(system_size);
      //Eigen::Matrix2cd H; H << 1, 1, 1, -1; H /= std::sqrt(2);
      //Eigen::Matrix2cd S; S << 1, 0, 0, 1j;
      //Eigen::Matrix2cd Y; Y << 0, -1j, 1j, 0;
      //sv4.QuantumState::evolve(H, 0u);
      //sv4.QuantumState::evolve(Y, 0u);
      //sv4.QuantumState::evolve(S, 0u);
      //sv4.QuantumState::evolve(Y, 0u);
      //sv4.QuantumState::evolve(S, 0u);
      //sv4.QuantumState::evolve(H, 0u);
      //sv4.QuantumState::evolve(S, 0u);
      std::cout << "sv1 (chp)          : " << sv1.to_string() << std::endl;
      std::cout << "sv2 (graphsim)     : " << sv2.to_string() << std::endl;
      std::cout << "sv3 (graphsim->chp): " << sv3.to_string() << std::endl;
      //std::cout << "sv4 (statevector)  : " << sv4.to_string() << std::endl;
      throw std::invalid_argument("States do not match.");
    }


    std::cout << "CHP state:   [ ";
    std::vector<int> s1 = state1.get_entropy_surface<int>();
    for (uint32_t j = 0; j < system_size; j++) {
      std::cout << s1[j] << " ";
    } std::cout << "]\n";

    std::cout << "Graph state: [ ";
    std::vector<int> s2 = state2.get_entropy_surface<int>();
    for (uint32_t j = 0; j < system_size; j++) {
      std::cout << s2[j] << " ";
    } std::cout << "]\n\n";


    for (uint32_t j = 0; j < system_size; j++) {
      if (s1[j] != s2[j]) {
        std::vector<uint32_t> qubits(j+1);
        std::iota(qubits.begin(), qubits.end(), 0);

        std::cout << state2.entropy(qubits, 0) << std::endl;

        std::string error_message = "Surfaces do not agree at step " + std::to_string(i) + ".";
        throw std::invalid_argument(error_message);
      }
    }
  }
}

using namespace dataframe;
using namespace dataframe::utils;

//#include <nanobind/nanobind.h>
//nanobind::bytes convert_bytes(const std::vector<dataframe::byte_t>& bytes) {
//  nanobind::bytes nb_bytes(bytes.data(), bytes.size());
//  return nb_bytes;
//}
//
//std::vector<dataframe::byte_t> convert_bytes(const nanobind::bytes& bytes) {
//  std::vector<dataframe::byte_t> bytes_vec(bytes.c_str(), bytes.c_str() + bytes.size());
//  bytes_vec.push_back('\0');
//  return bytes_vec;
//}

bool test_serialization() {
  std::string filename = "/Users/eliotheinrich/Projects/hypergraph/config.json";
  Params p = load_params(filename);

  std::cout << "running test_serialization\n";
  for (int i = 0; i < 1; i++) {
    TimeSamplingDriver<BlockSimulator> sampler(p);
    DataSlide s1 = sampler.generate_dataslide(1);

    std::vector<byte_t> bytes = s1.to_bytes();

    DataSlide s2(bytes);
    std::cout << s1.to_string() << std::endl;
    std::cout << s2.to_string() << std::endl;
  }

  return true;
}

int main() {
  assert(test_serialization());
}
