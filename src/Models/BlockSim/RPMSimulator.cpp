#include "RPMSimulator.h"

#define SUBSTRATE 0
#define PYRAMID 1

using namespace RPM_utils;

RPMSimulator::RPMSimulator(Params &params, uint32_t) : Simulator(params), sampler(params) {
  system_size = get<int>(params, "system_size");
  if (system_size < 1) {
    throw std::invalid_argument("System size must be larger than 0.");
  }

  pm = get<double>(params, "pm");
  pu = get<double>(params, "pu");

  pbc = get<int>(params, "pbc", false);
  initial_state = get<int>(params, "initial_state", SUBSTRATE);

  num_sites = 2*system_size;
  if (!pbc) {
    num_sites++;
  }

  params.emplace("u", pu/pm);

  start_sampling = false;

  surface = std::vector<int>(num_sites, 0);
  if (initial_state == SUBSTRATE) {
    for (uint32_t i = 0; i < num_sites; i++) {
      if (i % 2 == 1) {
        surface[i]++;
      }
    }
  } else if (initial_state == PYRAMID) {
    for (uint32_t i = 0; i < system_size; i++) {
      surface[i] = i;
      surface[num_sites - i - 1] = i + static_cast<int>(pbc);
    }

    if (!pbc) {
      surface[system_size] = system_size;
    }
  }
}

std::string RPMSimulator::to_string() const {
  std::string s = "[";
  std::vector<std::string> buffer;
  for (uint32_t i = 0; i < num_sites; i++) {
    buffer.push_back(std::to_string(surface[i]));
  }
  s += join(buffer, ", ") + "]";

  return s;
}

int RPMSimulator::slope(uint32_t i) const {
  return (surface[mod(i+1, num_sites)] - surface[mod(i-1, num_sites)])/2;
}

void RPMSimulator::peel(uint32_t i, bool right) {
  if (randf() > pm) {
    return;
  }

  int j = i;
  if (right) {
    while (surface[mod(j+1, num_sites)] > surface[i]) {
      j++;
    } 
    j++;
  } else {
    while (surface[mod(j-1, num_sites)] > surface[i]) {
      j--;
    } 
    j--;
  }

  int li = std::min(static_cast<int>(i), j);
  int ri = std::max(static_cast<int>(i), j);

  for (int k = li+1; k < ri; k++) {
    surface[mod(k, num_sites)] -= 2;
  }

  uint32_t size = (ri - li)/2;
  if (size > 0 && start_sampling) {
    sampler.record_size(size);
  }
}

void RPMSimulator::raise(uint32_t i) {
  if (randf() < pu) {
    surface[i] += 2;
  }
}

void RPMSimulator::timesteps(uint32_t num_steps) {
  for (uint32_t k = 0; k < num_steps; k++) {
    for (uint32_t i = 0; i < num_sites; i++) {
      uint32_t q = pbc ? rand() % num_sites : rand() % (num_sites - 2) + 1;

      int s = slope(q);
      if (!s && surface[q] > surface[mod(q - 1, num_sites)]) {
        // Tile hits a local maximum; do nothing
      } else if (!s && surface[q] < surface[mod(q - 1, num_sites)]) {
        // Tile hits a local minimum; raise
        raise(q);
      } else {
        // Hits a sloped section; peel
        peel(q, s > 0);
      }
    }
  }
}

data_t RPMSimulator::take_samples() {
  int N = pbc ? num_sites : num_sites - 1;
  std::vector<int> sampled_surface(surface.begin(), surface.begin() + N);

  data_t samples;
  sampler.add_samples(samples, sampled_surface);
  return samples;
}

