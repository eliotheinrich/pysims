#include "GroverSATSimulator.h"

#define DEFAULT_RECORD_FIDELITY false

using namespace dataframe;
using namespace dataframe::utils;

Clause::Clause(uint32_t x1, uint32_t x2, uint32_t x3, bool n1, bool n2, bool n3)
	: x1(x1), x2(x2), x3(x3), n1(n1), n2(n2), n3(n3) {
	if (x1 == x2 || x1 == x3 || x2 == x3) {
		throw std::invalid_argument("Invalid clause.");
	}
}

bool Clause::evaluate(bool v1, bool v2, bool v3) const {
	return (n1 ? !v1 : v1) || (n2 ? !v2 : v2) || (n3 ? !v3 : v3);
}

bool Clause::evaluate(const std::vector<bool>& vals) const {
	return evaluate(vals[x1], vals[x2], vals[x3]);
}

bool Clause::tautological() const {
	for (uint32_t v1 = 0; v1 < 2; v1++) {
		for (uint32_t v2 = 0; v2 < 2; v2++) {
			for (uint32_t v3 = 0; v3 < 2; v3++) {
				if (!evaluate(v1, v2, v3)) {
					return false;
				}
			}
		}
	}

	return true;
}

ConjugateNormalForm ConjugateNormalForm::random(uint32_t num_variables, uint32_t num_clauses, std::minstd_rand &gen) {
	std::vector<uint32_t> variables(num_variables);
	std::iota(variables.begin(), variables.end(), 0);

	std::uniform_int_distribution<> dist(0, num_variables - 1);	
	std::vector<Clause> clauses;
	for (uint32_t i = 0; i < num_clauses; i++) {
		uint32_t x1 = dist(gen);
		bool n1 = gen() % 2;

		bool valid = false;
		uint32_t x2;
		bool n2;
		while (!valid) {
			x2 = dist(gen);
			n2 = gen() % 2;

			if (x2 != x1) {
				valid = true;
			}
		}

		valid = false;
		uint32_t x3;
		bool n3;
		while (!valid) {
			x3 = dist(gen);
			n3 = gen() % 2;

			if (x3 != x1 && x3 != x2) {
				valid = true;
			}
		}

		clauses.push_back(Clause(x1, x2, x3, n1, n2, n3));
	}

	return ConjugateNormalForm{num_variables, num_clauses, clauses};
}

bool ConjugateNormalForm::evaluate(const std::vector<bool>& vals) const {
	if (vals.size() != num_variables) {
		throw std::invalid_argument("Number of variable assignments does not match number of variables.");
	}

	for (auto const& clause : clauses) {
		if (!clause.evaluate(vals)) {
			return false;
		}
	}

	return true;
}

GroverSATSimulator::GroverSATSimulator(ExperimentParams &params, uint32_t num_threads) : Simulator(params), sampler(params) {
	system_size = get<int>(params, "system_size");

	num_variables = get<int>(params, "num_variables");
	num_clauses = get<int>(params, "num_clauses");

	alpha = double(num_variables)/double(num_clauses);
	get<double>(params, "alpha", alpha);

	record_fidelity = get<int>(params, "record_fidelity", DEFAULT_RECORD_FIDELITY);

	Eigen::setNbThreads(num_threads);
	state = std::make_shared<Statevector>(system_size);
	hadamard_transform();

	cnf = ConjugateNormalForm::random(num_variables, num_clauses, rng);
	oracle = cnf_oracle(cnf);

	householder = generate_householder(system_size);
}

void GroverSATSimulator::hadamard_transform() {
	Eigen::MatrixXcd hadamard = Eigen::MatrixXcd(2, 2);
	hadamard << 1.0, 1.0, 1.0, -1.0;
	hadamard /= SQRT2;

	for (uint32_t i = 0; i < system_size; i++) {
		state->evolve(hadamard, std::vector<uint32_t>{i});
	}
}

void GroverSATSimulator::timesteps(uint32_t num_steps) {
	for (uint32_t i = 0; i < num_steps; i++) {
		state->evolve(oracle);
		hadamard_transform();
		state->evolve(householder);
		hadamard_transform();
	}
}

void GroverSATSimulator::add_fidelity_samples(SampleMap& samples) {
	uint32_t s = 1u << system_size;
	
	double p = 0;
	for (uint32_t i = 0; i < s; i++) {
		if (oracle(i).real() < 0) {
			p += std::pow(std::abs(state->data(i)), 2.0);
		}
	}

	samples.emplace("fidelity", p);
}

SampleMap GroverSATSimulator::take_samples() {
	SampleMap samples;
	sampler.add_samples(samples, state);

	if (record_fidelity) {
		add_fidelity_samples(samples);
	}

	return samples;
}
