#include "SandpileCliffordSimulator.h"
#include "RandomCliffordSimulator.hpp"

#define DEFAULT_BOUNDARY_CONDITIONS "pbc"
#define DEFAULT_FEEDBACK_MODE 22
#define DEFAULT_UNITARY_QUBITS false
#define DEFAULT_MZR_MODE 1

#define DEFAULT_SAMPLE_AVALANCHES false

#define SUBSTRATE 0
#define PYRAMID 1 
#define RANDOM 2

using namespace dataframe;
using namespace dataframe::utils;

SandpileCliffordSimulator::SandpileCliffordSimulator(ExperimentParams &params, uint32_t) : Simulator(params), interface_sampler(params), entropy_sampler(params) {
  system_size = get<int>(params, "system_size");

  mzr_prob = get<double>(params, "mzr_prob");
  unitary_prob = get<double>(params, "unitary_prob");
  params.emplace("u", unitary_prob/mzr_prob);

  boundary_condition = get<std::string>(params, "boundary_conditions", DEFAULT_BOUNDARY_CONDITIONS);
  feedback_mode = get<int>(params, "feedback_mode", DEFAULT_FEEDBACK_MODE);
  unitary_qubits = get<int>(params, "unitary_qubits", DEFAULT_UNITARY_QUBITS);
  mzr_mode = get<int>(params, "mzr_mode", DEFAULT_MZR_MODE);

  sample_avalanche_sizes = get<int>(params, "sample_avalanche_sizes", false);
  start_sampling = false;

  sample_reduced_surface = get<int>(params, "sample_reduced_surface", false);

  initial_state = get<int>(params, "initial_state", SUBSTRATE);
  scrambling_steps = get<int>(params, "scrambling_steps", system_size);

  // ------------------ TETRONIMOS -------------------
  // |  (1)  |  (2)  |  (3)  |  (4)  |  (5)  |  (6)  |
  // |       |       |       |       |       |     o |
  // |	   |   o   | o   o |     o |   o o |   o o |
  // | o o o | o o o | o o o | o o o | o o o | o o o |
  if (feedback_mode == 0)       feedback_strategy = std::vector<uint32_t>{1};
  else if (feedback_mode == 1)  feedback_strategy = std::vector<uint32_t>{1, 2};
  else if (feedback_mode == 2)  feedback_strategy = std::vector<uint32_t>{1, 3};
  else if (feedback_mode == 3)  feedback_strategy = std::vector<uint32_t>{1, 4};
  else if (feedback_mode == 4)  feedback_strategy = std::vector<uint32_t>{1, 5};
  else if (feedback_mode == 5)  feedback_strategy = std::vector<uint32_t>{1, 6};
  else if (feedback_mode == 6)  feedback_strategy = std::vector<uint32_t>{1, 2, 3};
  else if (feedback_mode == 7)  feedback_strategy = std::vector<uint32_t>{1, 2, 4};
  else if (feedback_mode == 8)  feedback_strategy = std::vector<uint32_t>{1, 2, 5};
  else if (feedback_mode == 9)  feedback_strategy = std::vector<uint32_t>{1, 2, 6};
  else if (feedback_mode == 10) feedback_strategy = std::vector<uint32_t>{1, 3, 4};
  else if (feedback_mode == 11) feedback_strategy = std::vector<uint32_t>{1, 3, 5};
  else if (feedback_mode == 12) feedback_strategy = std::vector<uint32_t>{1, 3, 6};
  else if (feedback_mode == 13) feedback_strategy = std::vector<uint32_t>{1, 4, 5};
  else if (feedback_mode == 14) feedback_strategy = std::vector<uint32_t>{1, 4, 6};
  else if (feedback_mode == 15) feedback_strategy = std::vector<uint32_t>{1, 5, 6};
  else if (feedback_mode == 16) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4};
  else if (feedback_mode == 17) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 5};
  else if (feedback_mode == 18) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 6};
  else if (feedback_mode == 19) feedback_strategy = std::vector<uint32_t>{1, 2, 4, 5};
  else if (feedback_mode == 20) feedback_strategy = std::vector<uint32_t>{1, 2, 4, 6};
  else if (feedback_mode == 21) feedback_strategy = std::vector<uint32_t>{1, 2, 5, 6};
  else if (feedback_mode == 22) feedback_strategy = std::vector<uint32_t>{1, 3, 4, 5};
  else if (feedback_mode == 23) feedback_strategy = std::vector<uint32_t>{1, 3, 4, 6};
  else if (feedback_mode == 24) feedback_strategy = std::vector<uint32_t>{1, 3, 5, 6};
  else if (feedback_mode == 25) feedback_strategy = std::vector<uint32_t>{1, 4, 5, 6};
  else if (feedback_mode == 26) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4, 5};
  else if (feedback_mode == 27) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4, 6};
  else if (feedback_mode == 28) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 5, 6};
  else if (feedback_mode == 29) feedback_strategy = std::vector<uint32_t>{1, 2, 4, 5, 6};
  else if (feedback_mode == 30) feedback_strategy = std::vector<uint32_t>{1, 3, 4, 5, 6};
  else if (feedback_mode == 31) feedback_strategy = std::vector<uint32_t>{1, 2, 3, 4, 5, 6};


  state = std::make_shared<QuantumCHPState>(system_size);

  if (initial_state == SUBSTRATE) {
    // Do nothing
  } else if (initial_state == PYRAMID) {
    bool offset = 0;

    for (uint32_t k = 0; k < scrambling_steps; k++) {
      rc_timestep(state, 2, offset, true);
      offset = !offset;
    }
  } else if (initial_state == RANDOM) {
    std::vector<uint32_t> all_sites(system_size);
    std::iota(all_sites.begin(), all_sites.end(), 0);
    state->random_clifford(all_sites);
  }
}

void SandpileCliffordSimulator::mzr(uint32_t i) {
  if (randf() < mzr_prob) {
    // (maybe) record entropy surface for avalanche calculations
    std::vector<int> entropy_surface1;
    if (sample_avalanche_sizes && start_sampling) {
      entropy_surface1 = state->get_entanglement<int>(2);
    }

    // Do measurement
    if (mzr_mode == 0) {
      state->mzr(i);
    } else if (mzr_mode == 1) {
      state->mzr(i);
      state->mzr(i+1);
    } else if (mzr_mode == 2) {
      if (randf() < 0.5) {
        state->mzr(i);
      } else {
        state->mzr(i+1);
      }
    }

    // record avalanche sizes
    if (sample_avalanche_sizes && start_sampling) {
      std::vector<int> entropy_surface2 = state->get_entanglement<int>(2);
      int s = 0.0;
      for (uint32_t i = 0; i < system_size; i++) {
        s += std::abs(entropy_surface1[i] - entropy_surface2[i]);
      }

      interface_sampler.record_size(s);
    }
  }
}

void SandpileCliffordSimulator::unitary(uint32_t i) {
  if (randf() < unitary_prob) {

    std::vector<uint32_t> qubits;
    if (unitary_qubits == 2) {
      qubits = std::vector<uint32_t>{i, i+1};
    } else if (unitary_qubits == 3) {
      qubits = std::vector<uint32_t>{i-1, i, i+1};
    } else if (unitary_qubits == 4) {
      qubits = std::vector<uint32_t>{i-1, i, i+1, i+2};
    }

    state->random_clifford(qubits);
  }
}

void SandpileCliffordSimulator::timesteps(uint32_t num_steps) {
  for (uint32_t k = 0; k < num_steps; k++) {
    timestep();
  }
}

void SandpileCliffordSimulator::left_boundary() {
  if (boundary_condition == "pbc") {
    std::vector<uint32_t> qubits{0, 1};
    state->random_clifford(qubits);
  } else if (boundary_condition == "obc1") {

  } else if (boundary_condition == "obc2") {
    std::vector<uint32_t> qubits{0, 1};
    state->random_clifford(qubits);
  } else { 
    std::string error_message = "Invalid boundary condition: " + boundary_condition;
    throw std::invalid_argument(error_message);
  }
}

void SandpileCliffordSimulator::right_boundary() {
  if (boundary_condition == "pbc") {
    std::vector<uint32_t> qubits{system_size-1, 0};
    state->random_clifford(qubits);
  } else if (boundary_condition == "obc1") {

  } else if (boundary_condition == "obc2") {
    std::vector<uint32_t> qubits{system_size-2, system_size-1};
    state->random_clifford(qubits);
  }
}

uint32_t SandpileCliffordSimulator::get_shape(uint32_t s0, uint32_t s1, uint32_t s2) const {
  int ds1 = s0 - s1;
  int ds2 = s2 - s1;

  if      ((ds1 == 0)  && (ds2 == 0))   return 1; // 0 0 0 (a)
  else if ((ds1 == -1) && (ds2 == -1))  return 2; // 0 1 0 (b)
  else if ((ds1 == 1)  && (ds2 == 1))   return 3; // 1 0 1 (c)
  else if ((ds1 == 0)  && (ds2 == 1))   return 4; // 0 0 1 (d)
  else if ((ds1 == 1)  && (ds2 == 0))   return 4; // 1 0 0
  else if ((ds1 == 0)  && (ds2 == -1))  return 5; // 1 1 0 (e)
  else if ((ds1 == -1) && (ds2 == 0))   return 5; // 0 1 1
  else if ((ds1 == -1) && (ds2 == 1))   return 6; // 0 1 2 (f)
  else if ((ds1 == 1)  && (ds2 == -1))  return 6; // 2 1 0
  else { 
    throw std::invalid_argument("Something has gone wrong with the entropy substrate."); 
  }
}

void SandpileCliffordSimulator::feedback(uint32_t q) {
  uint32_t q0;
  uint32_t q2;
  if (boundary_condition == "pbc") {
    q0 = mod(q - 1, system_size);
    q2 = mod(q + 1, system_size);
  } else {
    q0 = q - 1;
    q2 = q + 1;
  }

  int s0 = state->cum_entanglement<int>(q0);
  int s1 = state->cum_entanglement<int>(q);
  int s2 = state->cum_entanglement<int>(q2);

  uint32_t shape = get_shape(s0, s1, s2);

  if (std::count(feedback_strategy.begin(), feedback_strategy.end(), shape)) {
    unitary(q);
  } else {
    mzr(q);
  }
}

void SandpileCliffordSimulator::timestep() {
  left_boundary();

  std::uniform_int_distribution<> dis(1, system_size - 3);

  for (uint32_t i = 0; i < system_size; i++) {
    uint32_t q = dis(rng);
    feedback(q);
  }

  right_boundary();
}

void SandpileCliffordSimulator::add_reduced_substrate_height_samples(dataframe::SampleMap& samples, const std::vector<int>& entropy_surface) const {
  auto [e1, e2] = interface_sampler.find_edges(entropy_surface);
  std::vector<double> reduced_substrate(system_size, 0u);
  for (size_t i = e1; i < e2; i++) {
    reduced_substrate[i - e1] = static_cast<double>(entropy_surface[i]);
  }

  dataframe::utils::emplace(samples, "reduced_surface", reduced_substrate);
}

SampleMap SandpileCliffordSimulator::take_samples() {
  SampleMap samples;

  state->tableau.rref();

  std::vector<int> entropy_surface = state->get_entanglement<int>(2);

  interface_sampler.add_samples(samples, entropy_surface);
  if (sample_reduced_surface) {
    add_reduced_substrate_height_samples(samples, entropy_surface);
  }
  entropy_sampler.add_samples(samples, state);

  return samples;
}

template<>
struct glz::meta<SandpileCliffordSimulator> {
  static constexpr auto value = glz::object(
      "state", &SandpileCliffordSimulator::state
      );
};

void SandpileCliffordSimulator::deserialize(const std::vector<byte_t>& bytes) {
  auto parse_error = glz::read_beve(*this, bytes);
  if (parse_error) {
    throw std::runtime_error(fmt::format("Error deserializing SandpileCliffordSimulator: \n{}", glz::format_error(parse_error, bytes)));
  }
}

std::vector<byte_t> SandpileCliffordSimulator::serialize() const {
  std::vector<byte_t> data;
  auto write_error = glz::write_beve(*this, data);
  if (write_error) {
    throw std::runtime_error(fmt::format("Error serializing SandpileCliffordSimulator: \n{}", glz::format_error(write_error, data)));
  }
  return data;
}
