#include "QuantumState.h"

Statevector::Statevector(uint32_t num_qubits, uint32_t qregister) : QuantumState(num_qubits) {
	data = Eigen::VectorXcd::Zero(1u << num_qubits);
	data(qregister) = 1.;
}


Statevector::Statevector(uint32_t num_qubits) : Statevector(num_qubits, 0) {}

Statevector::Statevector(const QuantumCircuit &circuit) : Statevector(circuit.num_qubits) {
	evolve(circuit);
}

Statevector::Statevector(const Statevector& other) : Statevector(other.data) {}

Statevector::Statevector(const Eigen::VectorXcd& vec) : Statevector(std::log2(vec.size())) {
	uint32_t s = vec.size();
	if ((s & (s - 1)) != 0)
		throw std::invalid_argument("Provided data to Statevector does not have a dimension which is a power of 2.");

	data = vec;
}

Statevector::Statevector(const MatrixProductState& state) : Statevector(state.coefficients()) {}

std::string Statevector::to_string() const {
	uint32_t s = 1u << num_qubits;

	bool first = true;
	std::string st = "";
	for (uint32_t i = 0; i < s; i++) {
		if (std::abs(data(i)) > QS_ATOL) {
			std::string amplitude;
			if (std::abs(data(i).imag()) < QS_ATOL)
				amplitude = std::to_string(data(i).real());
			else
				amplitude = "(" + std::to_string(data(i).real()) + ", " + std::to_string(data(i).imag()) + ")";
			
			std::string bin = quantumstate_utils::print_binary(i, num_qubits);

			if (!first)
				st += " + ";
			first = false;
			st += amplitude + "|" + bin + ">";
		}
	}

	return st;
}

double Statevector::entropy(const std::vector<uint32_t> &qubits, uint32_t index) const {
	DensityMatrix rho(*this);
	return rho.entropy(qubits, index);
}

double Statevector::measure_probability(uint32_t q, bool outcome) const {
	uint32_t s = 1u << num_qubits;

	double prob_zero = 0.0;
	for (uint32_t i = 0; i < s; i++) {
		if (((i >> q) & 1u) == 0)
			prob_zero += std::pow(std::abs(data(i)), 2);
	}

	if (outcome)
		return 1.0 - prob_zero;
	else
		return prob_zero;
}

bool Statevector::measure(uint32_t q) {
	uint32_t s = 1u << num_qubits;

	double prob_zero = measure_probability(q, 0);

	uint32_t outcome = randf() < prob_zero;

	for (uint32_t i = 0; i < s; i++) {
		if (((i >> q) & 1u) != outcome)
			data(i) = 0.;
	}

	normalize();

	return outcome;
}

void Statevector::evolve(const Eigen::MatrixXcd &gate, const std::vector<uint32_t> &qubits) {
	uint32_t s = 1u << num_qubits;
	uint32_t h = 1u << qubits.size();
	if ((gate.rows() != h) || gate.cols() != h)
		throw std::invalid_argument("Invalid gate dimensions for provided qubits.");

	Eigen::VectorXcd ndata = Eigen::VectorXcd::Zero(s);

	for (uint32_t a1 = 0; a1 < s; a1++) {
		uint32_t b1 = quantumstate_utils::reduce_bits(a1, qubits);

		for (uint32_t b2 = 0; b2 < h; b2++) {
			uint32_t a2 = a1;
			for (uint32_t j = 0; j < qubits.size(); j++)
				a2 = quantumstate_utils::set_bit(a2, qubits[j], b2, j);

			ndata(a1) += gate(b1, b2)*data(a2);
		}
	}

	data = ndata;
}

void Statevector::evolve(const Eigen::MatrixXcd &gate) {
	if (!(gate.rows() == data.size() && gate.cols() == data.size()))
		throw std::invalid_argument("Invalid gate dimensions for provided qubits.");

	data = gate*data;
}

// Vector representing diagonal gate
void Statevector::evolve(const Eigen::VectorXcd &gate, const std::vector<uint32_t> &qubits) {
	uint32_t s = 1u << num_qubits;
	uint32_t h = 1u << qubits.size();

	if (gate.size() != h)
		throw std::invalid_argument("Invalid gate dimensions for provided qubits.");

	for (uint32_t a = 0; a < s; a++) {
		uint32_t b = quantumstate_utils::reduce_bits(a, qubits);

		data(a) *= gate(b);
	}
}

void Statevector::evolve(const Eigen::VectorXcd &gate) {
	uint32_t s = 1u << num_qubits;

	if (gate.size() != s)
		throw std::invalid_argument("Invalid gate dimensions for provided qubits.");

	for (uint32_t a = 0; a < s; a++)
		data(a) *= gate(a);
}

double Statevector::norm() const {
	double n = 0.;
	for (uint32_t i = 0; i < data.size(); i++)
		n += std::pow(std::abs(data(i)), 2);
	
	return std::sqrt(n);
}

void Statevector::normalize() {
	data = data/norm();
}

double Statevector::probabilities(uint32_t z, const std::vector<uint32_t>& qubits) const {
	uint32_t s = 1u << num_qubits;
	double p = 0.;
	for (uint32_t i = 0; i < s; i++) {

		if (quantumstate_utils::bits_congruent(i, z, qubits))
			p += std::pow(std::abs(data(i)), 2);
	}

	return p;
}

std::vector<double> Statevector::probabilities() const {
	uint32_t s = 1u << num_qubits;
	std::vector<double> probs(s);
	for (uint32_t i = 0; i < s; i++)
		probs[i] = std::pow(std::abs(data(i)), 2);

	return probs;
}

std::complex<double> Statevector::inner(const Statevector& other) const {
	uint32_t s = 1u << num_qubits;

	std::complex<double> c = 0.;
	for (uint32_t i = 0; i < s; i++)
		c += other.data(i)*std::conj(data(i));
	
	return c;
}


Eigen::VectorXd Statevector::svd(const std::vector<uint32_t>& qubits) const {
	Eigen::MatrixXcd matrix(data);

	uint32_t r = 1u << qubits.size();
	uint32_t c = 1u << (num_qubits - qubits.size());
	matrix.resize(r, c);

	Eigen::JacobiSVD<Eigen::MatrixXcd> svd(matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
	return svd.singularValues();
}