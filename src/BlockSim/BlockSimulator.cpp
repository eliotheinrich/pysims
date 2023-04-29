#include "BlockSimulator.h"

#define DEFAULT_RANDOM_SITE_SELECTION false
#define DEFAULT_FEEDBACK_MODE 22
#define DEFAULT_PRECUT false

#define DEFAULT_SAMPLE_AVALANCHE_SIZES true

#ifdef DEBUG
#define LOG(x) std::cout << x
#else
#define LOG(x)
#endif

//  (1)  |  (2)  |  (3)  |  (4)  |  (5)  |  (6)  |
//       |       |       |       |       | o     |
//       |   o   | o   o | o     | o o   | o o   |
// o o o | o o o | o o o | o o o | o o o | o o o |

BlockSimulator::BlockSimulator(Params &params) : Simulator(params), start_sampling(false) {
    system_size = params.get<int>("system_size");
    pm = params.get<float>("pm");
    pu = params.get<float>("pu");

    precut = params.get<int>("precut", DEFAULT_PRECUT);

    random_sites = params.get<int>("random_sites", DEFAULT_RANDOM_SITE_SELECTION);

    feedback_mode = params.get<int>("feedback_mode", DEFAULT_FEEDBACK_MODE);

	if      (feedback_mode == 0)  feedback_strategy = std::vector<uint>{1};
	else if (feedback_mode == 1)  feedback_strategy = std::vector<uint>{1, 2};
	else if (feedback_mode == 2)  feedback_strategy = std::vector<uint>{1, 3};
	else if (feedback_mode == 3)  feedback_strategy = std::vector<uint>{1, 4};
	else if (feedback_mode == 4)  feedback_strategy = std::vector<uint>{1, 5};
	else if (feedback_mode == 5)  feedback_strategy = std::vector<uint>{1, 6};
	else if (feedback_mode == 6)  feedback_strategy = std::vector<uint>{1, 2, 3};
	else if (feedback_mode == 7)  feedback_strategy = std::vector<uint>{1, 2, 4};
	else if (feedback_mode == 8)  feedback_strategy = std::vector<uint>{1, 2, 5};
	else if (feedback_mode == 9)  feedback_strategy = std::vector<uint>{1, 2, 6};
	else if (feedback_mode == 10) feedback_strategy = std::vector<uint>{1, 3, 4};
	else if (feedback_mode == 11) feedback_strategy = std::vector<uint>{1, 3, 5};
	else if (feedback_mode == 12) feedback_strategy = std::vector<uint>{1, 3, 6};
	else if (feedback_mode == 13) feedback_strategy = std::vector<uint>{1, 4, 5};
	else if (feedback_mode == 14) feedback_strategy = std::vector<uint>{1, 4, 6};
	else if (feedback_mode == 15) feedback_strategy = std::vector<uint>{1, 5, 6};
	else if (feedback_mode == 16) feedback_strategy = std::vector<uint>{1, 2, 3, 4};
	else if (feedback_mode == 17) feedback_strategy = std::vector<uint>{1, 2, 3, 5};
	else if (feedback_mode == 18) feedback_strategy = std::vector<uint>{1, 2, 3, 6};
	else if (feedback_mode == 19) feedback_strategy = std::vector<uint>{1, 2, 4, 5};
	else if (feedback_mode == 20) feedback_strategy = std::vector<uint>{1, 2, 4, 6};
	else if (feedback_mode == 21) feedback_strategy = std::vector<uint>{1, 2, 5, 6};
	else if (feedback_mode == 22) feedback_strategy = std::vector<uint>{1, 3, 4, 5};
	else if (feedback_mode == 23) feedback_strategy = std::vector<uint>{1, 3, 4, 6};
	else if (feedback_mode == 24) feedback_strategy = std::vector<uint>{1, 3, 5, 6};
	else if (feedback_mode == 25) feedback_strategy = std::vector<uint>{1, 4, 5, 6};
	else if (feedback_mode == 26) feedback_strategy = std::vector<uint>{1, 2, 3, 4, 5};
	else if (feedback_mode == 27) feedback_strategy = std::vector<uint>{1, 2, 3, 4, 6};
	else if (feedback_mode == 28) feedback_strategy = std::vector<uint>{1, 2, 3, 5, 6};
	else if (feedback_mode == 29) feedback_strategy = std::vector<uint>{1, 2, 4, 5, 6};
	else if (feedback_mode == 30) feedback_strategy = std::vector<uint>{1, 3, 4, 5, 6};

	sample_avalanche_sizes = params.get<int>("sample_avalanche_sizes", DEFAULT_SAMPLE_AVALANCHE_SIZES);
	avalanche_sizes = std::vector<uint>(system_size, 0);
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

uint BlockSimulator::get_shape(uint i) const {
	int s0 = surface[i-1];
	int s1 = surface[i];
	int s2 = surface[i+1];
	int ds1 = s0 - s1;
	int ds2 = s2 - s1;

	if      ((ds1 == 0)  && (ds2 == 0))   return 1;
	else if ((ds1 == -1) && (ds2 == -1))  return 2;
	else if ((ds1 == 1)  && (ds2 == 1))   return 3;
	else if ((ds1 == 0)  && (ds2 == 1))   return 4;
	else if ((ds1 == 1)  && (ds2 == 0))   return 4;
	else if ((ds1 == 0)  && (ds2 == -1))  return 5;
	else if ((ds1 == -1) && (ds2 == 0))   return 5;
	else if ((ds1 == -1) && (ds2 == 1))   return 6;
	else if ((ds1 == 1)  && (ds2 == -1))  return 6;
	else {
		LOG("Something has gone wrong at site " << i << "\n");
	}

	assert(false);
	return -1;
}

void BlockSimulator::avalanche(uint i) {
    if (precut && surface[i] > 0) surface[i]--;

    uint left = i;
    while (surface[left-1] > surface[i]) left--;

    uint right = i;
    while (surface[right+1] > surface[i]) right++;

	for (uint j = left; j < i; j++) surface[j]--;
	for (uint j = i+1; j < right+1; j++) surface[j]--;
    
    uint size = right - left;
    if (size > 0) {
        record_size(size);
    }
}

void BlockSimulator::stack(uint i) {
	uint shape = get_shape(i);
	if (shape == 1 || shape == 3 || shape == 4) surface[i]++;
}

void BlockSimulator::timesteps(uint num_steps) {
    for (uint i = 0; i < num_steps; i++) {
        for (uint i = 1; i < system_size - 1; i++) {
            uint q = random_sites ? rand() % (system_size - 2) + 1 : i;
            
            uint shape = get_shape(q);

            if (std::count(feedback_strategy.begin(), feedback_strategy.end(), shape)) {
                if (randf() < pu) stack(q);
            } else {
                if (randf() < pm) avalanche(q);
            }
        }
    }
}

void BlockSimulator::add_avalanche_samples(data_t &samples) {
    for (uint i = 0; i < system_size; i++) samples.emplace("avalanche_" + std::to_string(i), avalanche_sizes[i]);

    avalanche_sizes = std::vector<uint>(system_size, 0);
}

data_t BlockSimulator::take_samples() {
    data_t samples;
    for (uint i = 0; i < system_size; i++) samples.emplace("surface_" + std::to_string(i), surface[i]);

	if (sample_avalanche_sizes)
		add_avalanche_samples(samples);

    return samples;
}

