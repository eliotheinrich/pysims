#include "GroverProjectionSimulator.h"
#include <math.h>
#include <unsupported/Eigen/MatrixFunctions>

#define DEFAULT_NMAX 50

GroverProjectionSimulator::GroverProjectionSimulator(Params &params) : EntropySimulator(params) {
	mzr_prob = params.get<float>("mzr_prob");
	nmax = params.get<int>("nmax", DEFAULT_NMAX);

	offset = false;
}

void GroverProjectionSimulator::init_state() {
	state = std::unique_ptr<UnitaryState>(new UnitaryState(system_size));
}

void GroverProjectionSimulator::grover_projection(uint qubit) {
	auto probs = state->probabilities(qubit);
	if (randf() < probs.first)
		grover_projection(qubit, 0);
	else
		grover_projection(qubit, 1);
}

void GroverProjectionSimulator::grover_projection(uint qubit, bool outcome) {
	Eigen::MatrixXcd z(2, 2);
	z(0,0) = 1.; z(1, 1) = -1.;

	// TODO perform this operation inplace
	Eigen::MatrixXcd oracle = full_circuit_unitary(z, std::vector<uint>{qubit}, system_size);
	Eigen::MatrixXcd reflection = Eigen::MatrixXcd::Identity(1u << system_size, 1u << system_size);
	reflection(0,0) = -1;

	Eigen::MatrixXcd grover_op = state->unitary * reflection * state->unitary.adjoint() * oracle;

	auto p = state->probabilities(qubit);
	double theta = std::asin(std::sqrt(p.first));
	double max_amplitude = 0;
	uint projective_n = 0;

	for (uint n = 0; n < nmax; n++) {
		double amplitude = outcome ? std::cos((2*n + 1)*theta) : std::sin((2*n + 1)*theta);
		if (std::abs(amplitude) > max_amplitude) {
			projective_n = n;
			max_amplitude = amplitude;
		}
	}

	state->apply_gate(grover_op.pow(projective_n));

	// Large powers of the unitary matrix produce numerical instabilities;
	// After each iteration, normalize.
	state->normalize();
}

void GroverProjectionSimulator::timesteps(uint num_steps) {
	for (uint k = 0; k < num_steps; k++) {

		for (uint q = 0; q < system_size/2; q++) {
			uint q1 = offset ? 2*q : (2*q + 1) % system_size;
			uint q2 = offset ? (2*q + 1) % system_size : (2*q + 2) % system_size;

			std::vector<uint> qubits{q1, q2};

			state->apply_gate(haar_unitary(2, rng), qubits);
		}

		for (uint q = 0; q < system_size; q++) {
			if (randf() < mzr_prob)
				grover_projection(q);
		}

		offset = !offset;
	}
}