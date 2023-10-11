#pragma once

#include <DataFrame.hpp>
#include <Entropy.hpp>
#include <QuantumState.h>

#define SQRT2 1.41421356237

struct Clause {
	uint32_t x1;
	uint32_t x2;
	uint32_t x3;

	bool n1;
	bool n2;
	bool n3;

	Clause() {}
	Clause(uint32_t x1, uint32_t x2, uint32_t x3, bool n1, bool n2, bool n3);
	bool evaluate(bool v1, bool v2, bool v3) const;
	bool evaluate(const std::vector<bool>& vals) const;
	bool tautological() const;
};

struct ConjugateNormalForm {
	uint32_t num_variables;
	uint32_t num_clauses;
	std::vector<Clause> clauses;

	ConjugateNormalForm() {}
	ConjugateNormalForm(uint32_t num_variables, uint32_t num_clauses, std::vector<Clause> clauses) : num_variables(num_variables), num_clauses(num_clauses), clauses(clauses) {}
	ConjugateNormalForm(uint32_t num_variables, uint32_t num_clauses) : ConjugateNormalForm(num_variables, num_clauses, std::vector<Clause>()) {}

	static ConjugateNormalForm random(uint32_t num_variables, uint32_t num_clauses, std::minstd_rand &gen);

	bool evaluate(const std::vector<bool>& vals) const;
};

static Eigen::VectorXcd cnf_oracle(const ConjugateNormalForm& cnf) {
	uint32_t num_variables = cnf.num_variables;
	uint32_t s = 1u << num_variables;

	Eigen::VectorXcd oracle = Eigen::VectorXcd::Zero(s);

	for (uint32_t i = 0; i < s; i++) {
		std::vector<bool> assignments(num_variables);
		for (uint32_t j = 0; j < num_variables; j++) {
			assignments[j] = (i >> j) & 1u;
		}

		oracle(i) = -(2*int(cnf.evaluate(assignments)) - 1);
	}

	return oracle;
}

static Eigen::VectorXcd generate_householder(uint32_t num_qubits) {
	Eigen::VectorXcd householder(1u << num_qubits);
	householder.setOnes();
	householder(0) = -1;

	return -householder;
}

class GroverSATSimulator : public EntropySimulator {
	private:
		double alpha;

		uint32_t num_variables;
		uint32_t num_clauses;

		bool record_fidelity;

		ConjugateNormalForm cnf;
		Eigen::VectorXcd oracle;
		Eigen::VectorXcd householder;

		void hadamard_transform();

	public:
		std::shared_ptr<Statevector> state;

		GroverSATSimulator(Params &params);

		virtual void init_state(uint32_t num_threads) override;
		virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override { return state->entropy(qubits, index); }
		virtual void timesteps(uint32_t num_steps) override;

		void add_fidelity_samples(data_t& samples);
		virtual data_t take_samples() override;

		CLONE(Simulator, GroverSATSimulator)
};