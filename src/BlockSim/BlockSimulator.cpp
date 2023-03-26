#include "BlockSimulator.h"

static AvalancheType parse_avalanche_type(std::string s) {
    if      (s == "waterline") return AvalancheType::Waterline;
    else if (s == "uphill") return AvalancheType::Uphill;
    else {
        std::cout << "Unsupported avalanche type: " << s << std::endl;
        assert(false);
    }
}

BlockSimulator::BlockSimulator(Params &params) : Simulator(params) {
    system_size = params.get<int>("system_size");
    pm = params.get<float>("pm");
    pu = params.get<float>("pu");

    random_sites = params.get<int>("random_sites", DEFAULT_RANDOM_SITE_SELECTION);
    avalanche_type = parse_avalanche_type(params.get<std::string>("avalanche_type", DEFAULT_AVALANCHE_TYPE));
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

void BlockSimulator::unitary_stack(uint i) {
    surface[i]++;
    if ((std::abs(slope(i)) > 1) || (std::abs(slope(i-1)) > 1)) surface[i]--;
}

void BlockSimulator::unitary_timestep() {
//std::cout << "Before unitary stacking: " << to_string() << std::endl;
    for (uint i = 1; i < system_size - 1; i++) {
        uint q;
        if (random_sites) q = rand() % (system_size - 2) + 1;
        else q = i;
        //std::cout << "qu = " << q << std::endl;
        
        if (randf() < pu) unitary_stack(q);
    }
//std::cout << "After unitary: " << to_string() << std::endl;
}

void BlockSimulator::waterline_avalanche(uint i) {
    uint j = i+1;
    while (surface[j] > surface[i]) {
        surface[j]--;
        j++;
    }

    uint size = j - i;
    avalanche_sizes.push_back(size);
}

void BlockSimulator::uphill_avalanche(uint i) {
    uint j = i+1;
    while (slope(j) == 1) j++;
    for (uint k = i+1; k < j+1; k++) surface[k]--;

    uint size = j - i;
    avalanche_sizes.push_back(size);
}

void BlockSimulator::projective_timestep() {
    for (uint i = 1; i < system_size - 1; i++) {
        uint q;
        if (random_sites) q = rand() % (system_size - 2) + 1;
        else q = i;

        //std::cout << "qp = " << q << std::endl;
        if ((slope(q) == 1) && randf() < pm) {
            switch (avalanche_type) {
                case (AvalancheType::Waterline): waterline_avalanche(q); break;
                case (AvalancheType::Uphill): uphill_avalanche(q); break;
            }
        }
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

