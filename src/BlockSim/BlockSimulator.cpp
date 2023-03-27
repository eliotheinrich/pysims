#include "BlockSimulator.h"

BlockSimulator::BlockSimulator(Params &params) : Simulator(params) {
    system_size = params.get<int>("system_size");
    pm = params.get<float>("pm");
    pu = params.get<float>("pu");

    random_sites = params.get<int>("random_sites", DEFAULT_RANDOM_SITE_SELECTION);
}

void BlockSimulator::init_state() {
    surface = std::vector<uint>(system_size, 0);
}

std::string BlockSimulator::to_string() const {
    std::string s = "[";
    std::vector<std::string> buffer;
    for (uint i = 0; i < system_size; i++) buffer.push_back(std::to_string(surface[i]));
    s += join(buffer, ", ") + "]";

    return s;
}

int BlockSimulator::slope(uint i) const {
    assert(i < system_size-1);
    return surface[i+1] - surface[i];
}

void BlockSimulator::avalanche(uint i) {
    uint s1 = surface[i-1];
    uint s2 = surface[i];
    uint s3 = surface[i+1];

    int ds1 = s1 - s2;
    int ds2 = s3 - s2;

	if       ((ds1 == 0) && (ds2 == 0))   {  // (1)

    } else if ((ds1 == -1) && (ds2 == -1)) { // (2)
        surface[i]--;
        avalanche_sizes.push_back(1);
    } else if ((ds1 == 1) && (ds2 == 1)) {  // (3)
        // Both directions?
    } else if ((ds1 == 0) && (ds2 == 1)) {  // (4)

    } else if ((ds1 == 1) && (ds2 == 0)) {  // (4)

    } else if ((ds1 == 0) && (ds2 == -1)) { // (5)
        surface[i]--;
        avalanche_sizes.push_back(1);
    } else if ((ds1 == -1) && (ds2 == 0)) { // (5)
        surface[i]--;
		avalanche_sizes.push_back(1);
    } else if ((ds1 == -1) && (ds2 == 1)) { // (6)
        uint j = i + 1;
        while (surface[j] > surface[i]) {
            surface[j]--;
            j++;
        }
        avalanche_sizes.push_back(j - i);
    } else if ((ds1 == 1) && (ds2 == -1)) { // (6)
        uint j = i - 1;
        while (surface[j] > surface[i]) {
            surface[j]--;
            j--;
        }

        avalanche_sizes.push_back(i - j);
    }
}

void BlockSimulator::projective_timestep() {
    for (uint i = 1; i < system_size - 1; i++) {
        uint q;
        if (random_sites) q = rand() % (system_size - 2) + 1;
        else q = i;

        if (randf() < pm) avalanche(q);
    }
}

void BlockSimulator::unitary_stack(uint i) {
    surface[i]++;
    if ((std::abs(slope(i)) > 1) || (std::abs(slope(i-1)) > 1)) surface[i]--;
}

void BlockSimulator::unitary_timestep() {
    for (uint i = 1; i < system_size - 1; i++) {
        uint q;
        if (random_sites) q = rand() % (system_size - 2) + 1;
        else q = i;
        
        if (randf() < pu) unitary_stack(q);
    }
}

void BlockSimulator::timesteps(uint num_steps) {
    for (uint i = 0; i < num_steps; i++) {
        unitary_timestep();
        projective_timestep();
    }
}

std::map<std::string, Sample> BlockSimulator::take_samples() {
    std::map<std::string, Sample> data;
    for (uint i = 0; i < system_size; i++) data.emplace("surface_" + std::to_string(i), surface[i]);

    std::vector<uint> sizes(system_size, 0);
    for (auto const s : avalanche_sizes) sizes[s]++;
    for (uint i = 0; i < system_size; i++) data.emplace("avalanche_" + std::to_string(i), sizes[i]);

    avalanche_sizes.clear();

    return data;
}

