#include "BlockSimulator.h"

BlockSimulator::BlockSimulator(Params &params) : Simulator(params) {
    system_size = params.geti("system_size");
    prob = params.getf("prob");

    avalanche_size = Sample();
    random_sites = params.geti("random_sites", DEFAULT_RANDOM_SITE_SELECTION);
    avalanche_type = params.geti("avalanche_type", DEFAULT_AVALANCHE_TYPE);
}

void BlockSimulator::init_state() {
    surface = new std::vector<uint>(system_size, 0);
}

std::string BlockSimulator::to_string() const {
    std::string s = "[";
    std::vector<std::string> buffer;
    for (uint i = 0; i < system_size; i++) buffer.push_back(std::to_string((*surface)[i]));
    s += join(buffer, ", ") + "]";

    return s;
}

int BlockSimulator::slope(uint i) const {
    assert(i < system_size-1);
    return (*surface)[i+1] - (*surface)[i];
}

void BlockSimulator::unitary_stack(uint i) {
    (*surface)[i]++;
    if ((std::abs(slope(i)) > 1) || (std::abs(slope(i-1)) > 1)) (*surface)[i]--;
}

void BlockSimulator::unitary_timestep() {
//std::cout << "Before unitary stacking: " << to_string() << std::endl;
    for (uint i = 1; i < system_size - 1; i++) {
        uint q;
        if (random_sites) q = rand() % (system_size - 2) + 1;
        else q = i;
        
        unitary_stack(q);
    }
//    offset = !offset;
//std::cout << "After unitary: " << to_string() << std::endl;
}

void BlockSimulator::waterline_avalanche(uint i) {
    uint j = i+1;
    while ((*surface)[j] > (*surface)[i]) {
        (*surface)[j]--;
        j++;
    }

    uint size = j - i;
    avalanche_size = avalanche_size.combine(Sample(size));
}

void BlockSimulator::uphill_avalanche(uint i) {
    uint j = i+1;
    while (slope(j) == 1) j++;
    for (uint k = i+1; k < j+1; k++) (*surface)[k]--;

    uint size = j - i;
    avalanche_size = avalanche_size.combine(Sample(size));
}

void BlockSimulator::projective_timestep() {
//std::cout << "Before projective timestep\n";
    if (avalanche_type == 2) {
        uint q = rand() % (system_size - 2) + 1;
        waterline_avalanche(q);
        return;
    }

    for (uint i = 1; i < system_size - 1; i++) {
        uint q;
        if (random_sites) q = rand() % (system_size - 2) + 1;
        else q = i;

        if (slope(q) == 1) {
            if (randf() < prob) {
                if (avalanche_type == 0) waterline_avalanche(q);
                else if (avalanche_type == 1) uphill_avalanche(q);
            }
        }
    }
//std::cout << "After projective timestep\n";
}

void BlockSimulator::timesteps(uint num_steps) {
    for (uint i = 0; i < num_steps; i++) {
//std::cout << "Timestep " << i << ": " << to_string() << std::endl;
        unitary_timestep();
        projective_timestep();
    }
}

std::map<std::string, Sample> BlockSimulator::take_samples() {
    std::map<std::string, Sample> data;
    data.emplace("avalanche_size", avalanche_size);
    for (uint i = 0; i < system_size; i++) {
        data.emplace("surface_" + std::to_string(i), Sample((*surface)[i]));
    }

    return data;
}

