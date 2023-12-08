#include "GroverProjectionSimulator.h"
#include <math.h>

#define DEFAULT_NMAX 500
#define DEFAULT_PROJECTION_TYPE "single"

#define DEFAULT_EPS 0.0

GroverProjectionSimulator::GroverProjectionSimulator(Params &params, uint32_t num_threads) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");
	mzr_prob = get<double>(params, "mzr_prob");
	nmax = get<int>(params, "nmax", DEFAULT_NMAX);

	projection_type = get<std::string>(params, "projection_type", DEFAULT_PROJECTION_TYPE);
	eps = get<double>(params, "eps", DEFAULT_EPS);

	dist = std::binomial_distribution<uint32_t>(system_size, mzr_prob);

	offset = false;

	Eigen::setNbThreads(num_threads);
	state = std::make_shared<UnitaryState>(system_size);
}

Eigen::MatrixXcd GroverProjectionSimulator::create_oracle(uint32_t z, const std::vector<uint32_t>& qubits) const {
	uint32_t s = 1u << system_size;

	Eigen::MatrixXcd oracle = Eigen::MatrixXcd::Identity(s, s);

	for (uint32_t i = 0; i < s; i++) {
		if (quantumstate_utils::bits_congruent(i, z, qubits)) {
			oracle(i, i) = -1;
		}
	}

	return oracle;
}

Eigen::MatrixXcd GroverProjectionSimulator::create_oracle(uint32_t z) const {
	std::vector<uint32_t> qubits(system_size);
	std::iota(qubits.begin(), qubits.end(), 0);

	return create_oracle(z, qubits);
}


void GroverProjectionSimulator::grover_projection(uint32_t z, const std::vector<uint32_t>& qubits) {
	assert(z < (1u << qubits.size()));
	Eigen::MatrixXcd oracle = create_oracle(z, qubits);

	Eigen::MatrixXcd reflection = create_oracle(0);

	Eigen::MatrixXcd grover_op = state->unitary * reflection * state->unitary.adjoint() * oracle;

	double p = state->probabilities(z, qubits);
	double theta = std::asin(std::sqrt(p));

	double min_err = 1.0;
	uint32_t projective_n = 0;

	uint32_t n = 0;
	double err = 1.0;
	while (n++ < nmax && err > eps) {
		err = 1.0 - std::abs(std::sin((2*n + 1)*theta));
		if (err < min_err) {
			projective_n = n;
			min_err = err;
		}
	}

	state->evolve(grover_op.pow(projective_n));

	// Large powers of the unitary matrix produce numerical instabilities;
	// After each iteration, normalize.
	state->normalize();
}

void GroverProjectionSimulator::grover_projection(uint32_t qubit) {
	double p0 = state->probabilities(0, std::vector<uint32_t>{qubit});
	uint32_t z = !(randf() < p0);

	grover_projection(z, std::vector<uint32_t>{qubit});
}

void GroverProjectionSimulator::random_grover_projection() {
	uint32_t n = dist(rng);

	std::vector<uint32_t> all_qubits(system_size);
	std::iota(all_qubits.begin(), all_qubits.end(), 0);

	std::vector<uint32_t> qubits;
	std::sample(all_qubits.begin(), all_qubits.end(), std::back_inserter(qubits), n, rng);

	uint32_t z = rng() % (1u << n);

	grover_projection(z, qubits);
}

void GroverProjectionSimulator::timesteps(uint32_t num_steps) {
	for (uint32_t k = 0; k < num_steps; k++) {
		for (uint32_t q = 0; q < system_size/2; q++) {
			uint32_t q1 = offset ? 2*q : (2*q + 1) % system_size;
			uint32_t q2 = offset ? (2*q + 1) % system_size : (2*q + 2) % system_size;

			std::vector<uint32_t> qubits{q1, q2};

			state->evolve(haar_unitary(2), qubits);
		}

		if (projection_type == "multi") {
			random_grover_projection();
		} else if (projection_type == "single") {
			for (uint32_t q = 0; q < system_size; q++) {
				if (randf() < mzr_prob) {
					grover_projection(q);
				}
			}
		}

		offset = !offset;
	}
}