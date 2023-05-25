#ifndef QSTATEVECTOR_STATE
#define QSTATEVECTOR_STATE

#include <Eigen/Dense>
#include <random>

#define EPS 1e-8

static Eigen::MatrixXcd haar_unitary(uint num_qubits, std::minstd_rand &rng) {
	Eigen::MatrixXcd z = Eigen::MatrixXcd::Zero(1u << num_qubits, 1u << num_qubits);
	std::normal_distribution<double> distribution(0.0, 1.0);

	for (uint r = 0; r < z.rows(); r++) {
		for (uint c = 0; c < z.cols(); c++)
			z(r, c) = std::complex<double>(distribution(rng), distribution(rng));
	}

	Eigen::MatrixXcd q, r;
	Eigen::HouseholderQR<Eigen::MatrixXcd> qr(z);

	q = qr.householderQ();
	r = qr.matrixQR().triangularView<Eigen::Upper>();

	Eigen::MatrixXcd d = Eigen::MatrixXcd::Zero(1u << num_qubits, 1u << num_qubits);
	d.diagonal() = r.diagonal().cwiseQuotient(r.diagonal().cwiseAbs());

	return q * d;
}

static Eigen::MatrixXcd haar_unitary(uint num_qubits) {
	thread_local std::minstd_rand rng;
	return haar_unitary(num_qubits, rng);
}


static std::string print_binary(uint a, uint width=5) {
	std::string s = "";
	for (uint i = 0; i < width; i++)
		s = std::to_string((a >> i) & 1u) + s;
	return s;
}

static Eigen::MatrixXcd full_circuit_unitary(const Eigen::MatrixXcd &gate, const std::vector<uint> &qubits, uint total_qubits) {
	assert(total_qubits >= qubits.size());
	assert(1u << qubits.size() == gate.rows() && 1u << qubits.size() == gate.cols());

	uint s = 1u << total_qubits;
	uint h = 1u << qubits.size();

	Eigen::MatrixXcd full_gate = Eigen::MatrixXcd::Zero(s, s);
	for (uint i = 0; i < s; i++) {
		uint r = 0;
		for (uint k = 0; k < qubits.size(); k++) {
			uint x = (i >> qubits[k]) & 1u;
			uint p = qubits.size() - k - 1;
			r = (r & ~(1u << p)) | (x << p);
		}

		for (uint c = 0; c < h; c++) {
			uint j = i;
			// j is total bits
			// c is subsystem bit
			// set the q[k]th bit of j equal to the kth bit of c
			for (uint k = 0; k < qubits.size(); k++) {
				uint p = qubits.size() - k - 1;
				uint x = (c >> p) & 1u;
				j = (j & ~(1u << qubits[k])) | (x << qubits[k]);
			}

			full_gate(i, j) = gate(r, c);
		}
	}

	return full_gate;
}

static Eigen::MatrixXcd kronecker_product(const Eigen::MatrixXcd& A, const Eigen::MatrixXcd& B) {
    int rows_A = A.rows();
    int cols_A = A.cols();
    int rows_B = B.rows();
    int cols_B = B.cols();

    int rows_C = rows_A * rows_B;
    int cols_C = cols_A * cols_B;

    Eigen::MatrixXcd C = Eigen::MatrixXcd::Zero(rows_C, cols_C);

    for (int i = 0; i < rows_A; ++i) {
        for (int j = 0; j < cols_A; ++j) {
            C.block(i * rows_B, j * cols_B, rows_B, cols_B) = A(i, j) * B;
        }
    }

    return C;
}

static Eigen::MatrixXcd density_matrix(const Eigen::VectorXcd &statevector) {
	return kronecker_product(statevector.adjoint(), statevector);
}

static Eigen::MatrixXcd partial_trace(const Eigen::MatrixXcd &rho, const std::vector<uint> &traced_qubits) {
	uint num_qubits = std::log2(rho.rows());

	for (auto const &q : traced_qubits)
		assert(q >= 0 && q < num_qubits);

	std::vector<uint> remaining_qubits;
	for (uint q = 0; q < num_qubits; q++) {
		if (!std::count(traced_qubits.begin(), traced_qubits.end(), q))
			remaining_qubits.push_back(q);
	}

	uint num_traced_qubits = traced_qubits.size();
	uint num_remaining_qubits = remaining_qubits.size();

	Eigen::MatrixXcd reduced_rho = Eigen::MatrixXcd::Zero(1u << num_remaining_qubits, 1u << num_remaining_qubits);

	uint s = 1u << num_traced_qubits;
	uint h = 1u << num_remaining_qubits;

	for (uint r = 0; r < h; r++) {
		for (uint c = 0; c < h; c++) {
			// Indices into full rho
			uint i = 0;
			uint j = 0;
			for (uint k = 0; k < num_remaining_qubits; k++) {
				// Set bits in (i,j) corresponding to remaining qubits
				uint q = remaining_qubits[k];
				uint x = (r >> k) & 1u;
				i = (i & ~(1u << q)) | (x << q);
				uint y = (c >> k) & 1u;
				j = (j & ~(1u << q)) | (y << q);
			}

			for (uint n = 0; n < s; n++) {
				for (uint k = 0; k < num_traced_qubits; k++) {
					// Set bits in (i,j) corresponding to traced qubits
					uint q = traced_qubits[k];
					uint x = (n >> k) & 1u;
					i = (i & ~(1u << q)) | (x << q);
					j = (j & ~(1u << q)) | (x << q);
				}

				//std::cout << r << ", " << c << ", " << i << ", " << j << std::endl;
				reduced_rho(r, c) += rho(i, j);
			}
		}
	}

	return reduced_rho;
}

static Eigen::MatrixXcd normalize_unitary(Eigen::MatrixXcd &unitary) {
	auto QR = unitary.householderQr();
	Eigen::MatrixXcd Q = QR.householderQ();

	return Q;
}

class QuantumStatevector {
	private:
		uint num_qubits;

	public:
		Eigen::MatrixXcd data;
		QuantumStatevector(uint num_qubits) : num_qubits(num_qubits) {
			data = Eigen::VectorXcd::Zero(1u << num_qubits);
			data(0) = 1.;
		}

		std::string to_string() const {
			uint s = 1u << num_qubits;

			bool first = true;
			std::string st = "";
			for (uint i = 0; i < s; i++) {
				if (std::abs(data(i)) > EPS) {
					std::string amplitude;
					if (std::abs(data(i).imag()) < EPS)
						amplitude = std::to_string(data(i).real());
					else
						amplitude = "(" + std::to_string(data(i).real()) + ", " + std::to_string(data(i).imag()) + ")";
					
					std::string bin = print_binary(i, num_qubits);

					if (!first)
						st += " + ";
					first = false;
					st += amplitude + "|" + bin + ">";
				}
			}

			return st;
		}

		void apply_gate(const Eigen::MatrixXcd &gate, const std::vector<uint> &qubits) {
			apply_gate(full_circuit_unitary(gate, qubits, num_qubits));
		}

		void apply_gate(const Eigen::MatrixXcd &gate) {
			assert(gate.rows() == data.size() && gate.cols() == data.size());
			data = gate*data;
		}

		double norm() const {
			double n = 0.;
			for (uint i = 0; i < data.size(); i++)
				n += std::pow(std::abs(data(i)), 2);
			
			return std::sqrt(n);
		}

		void normalize() {
			data = data/norm();
		}

		float entropy(const std::vector<uint> &qubits) const {
			for (auto const &q : qubits)
				assert(q >= 0 && q < num_qubits);

			Eigen::MatrixXcd rho = density_matrix(data);

			std::vector<uint> traced_qubits;
			for (uint q = 0; q < num_qubits; q++) {
				if (!std::count(qubits.begin(), qubits.end(), q))
					traced_qubits.push_back(q);
			}

			Eigen::MatrixXcd rho_a = partial_trace(rho, traced_qubits);

			// For now, just compute 2nd Renyi entropy
			return -std::log2((rho_a * rho_a).trace().real());
		}

		std::pair<double, double> probabilities(uint qubit) const {
			uint s = 1u << num_qubits;
			double p0 = 0.;
			double p1 = 0.;
			for (uint i = 0; i < s; i++) {
				if ((i >> qubit) & 1u)
					p1 += std::pow(std::abs(data(i)), 2);
				else
					p0 += std::pow(std::abs(data(i)), 2);
			}

			return std::pair<double, double>{p0, p1};
		}
};

#endif