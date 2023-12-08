#pragma once

#include <Eigen/Dense>
#include <unsupported/Eigen/MatrixFunctions>
#include <vector>
#include <variant>
#include <string>
#include <complex>
#include <memory>

// --- Definitions for gates/measurements --- //

#define GATECLONE(A) virtual std::shared_ptr<Gate> clone() override { return std::shared_ptr<Gate>(new A(qbits)); }

class Gate {
	public:
		std::vector<uint32_t> qbits;
		uint32_t num_qubits;

		Gate(const std::vector<uint32_t>& qbits)
		: qbits(qbits), num_qubits(qbits.size()) {
			assert(qargs_unique(qbits));
		}

		virtual uint32_t num_params() const=0;
		virtual std::string label() const=0;
		virtual Eigen::MatrixXcd define(const std::vector<double>& params) const=0;
		Eigen::MatrixXcd define() const {
			if (num_params() > 0) {
				throw std::invalid_argument("Unbound parameters; cannot define gate.");
			}
		
			return define(std::vector<double>());
		}
		Eigen::MatrixXcd adjoint(const std::vector<double>& params) const {
			return define(params).adjoint();
		}
		Eigen::MatrixXcd adjoint() const {
			if (num_params() > 0) {
				throw std::invalid_argument("Unbound parameters; cannot define gate.");
			}

			return adjoint(std::vector<double>());
		}
		virtual std::shared_ptr<Gate> clone()=0;
};

class MatrixGate : public Gate {
	public:
		Eigen::MatrixXcd data;
		std::string label_str;

		MatrixGate(const Eigen::MatrixXcd& data, const std::vector<uint32_t>& qbits, std::string label_str)
			: Gate(qbits), data(data), label_str(label_str) {}

		MatrixGate(const Eigen::MatrixXcd& data, const std::vector<uint32_t>& qbits)
			: MatrixGate(data, qbits, "U") {}

		
		uint32_t num_params() const override {
			return 0;
		}

		std::string label() const override {
			return label_str;
		}

		Eigen::MatrixXcd define(const std::vector<double>& params) const override {
			if (params.size() != 0) {
				throw std::invalid_argument("Cannot pass parameters to MatrixGate.");
			}
			return data;
		}

		std::shared_ptr<Gate> clone() override { 
			return std::shared_ptr<Gate>(new MatrixGate(data, qbits)); 
		}
};

class Measurement {
	public:
		std::vector<uint32_t> qbits;
		Measurement(const std::vector<uint32_t>& qbits) : qbits(qbits) {}
};

typedef std::variant<std::shared_ptr<Gate>, Measurement> Instruction;









// Defining types of gates

class RxRotationGate : public Gate {
	public:
		RxRotationGate(const std::vector<uint32_t>& qbits) : Gate(qbits) {
			if (qbits.size() != 1)
				throw std::invalid_argument("Rx gate can only have a single qubit.");
		}

		virtual uint32_t num_params() const override { return 1; }
		virtual std::string label() const override { return "Rx"; }

		virtual Eigen::MatrixXcd define(const std::vector<double>& params) const override {
			if (params.size() != num_params())
				throw std::invalid_argument("Invalid number of params passed to RxRotationGate.define().");

			Eigen::MatrixXcd gate = Eigen::MatrixXcd::Zero(2, 2);

			double t = params[0];
			gate << std::complex<double>(std::cos(t/2), 0), std::complex<double>(0, -std::sin(t/2)), 
				    std::complex<double>(0, -std::sin(t/2)), std::complex<double>(std::cos(t/2), 0);
			return gate;
		}

		GATECLONE(RxRotationGate);
};

class RyRotationGate : public Gate {
	public:
		RyRotationGate(const std::vector<uint32_t>& qbits) : Gate(qbits) {
			if (qbits.size() != 1)
				throw std::invalid_argument("Ry gate can only have a single qubit.");
		}

		virtual uint32_t num_params() const override { return 1; }
		virtual std::string label() const override { return "Ry"; }

		virtual Eigen::MatrixXcd define(const std::vector<double>& params) const override {
			if (params.size() != num_params())
				throw std::invalid_argument("Invalid number of params passed to RyRotationGate.define().");

			Eigen::MatrixXcd gate = Eigen::MatrixXcd::Zero(2, 2);

			double t = params[0];
			gate << std::complex<double>(std::cos(t/2), 0), std::complex<double>(-std::sin(t/2), 0), 
				    std::complex<double>(std::sin(t/2), 0), std::complex<double>(std::cos(t/2), 0);
			return gate;
		}

		GATECLONE(RyRotationGate);
};

class RzRotationGate : public Gate {
	public:
		RzRotationGate(const std::vector<uint32_t>& qbits) : Gate(qbits) {
			if (qbits.size() != 1)
				throw std::invalid_argument("Rz gate can only have a single qubit.");
		}

		virtual uint32_t num_params() const override { return 1; }
		virtual std::string label() const override { return "Rz"; }

		virtual Eigen::MatrixXcd define(const std::vector<double>& params) const override {
			if (params.size() != num_params())
				throw std::invalid_argument("Invalid number of params passed to RzRotationGate.define().");

			Eigen::MatrixXcd gate = Eigen::MatrixXcd::Zero(2, 2);

			double t = params[0];
			gate << std::complex<double>(std::cos(t/2), -std::sin(t/2)), std::complex<double>(0.0, 0.0), 
				    std::complex<double>(0.0, 0.0), std::complex<double>(std::cos(t/2), std::sin(t/2));
			return gate;
		}

		GATECLONE(RzRotationGate);
};

static std::shared_ptr<Gate> parse_gate(const std::string& s, const std::vector<uint32_t>& qbits) {
	Eigen::MatrixXcd data(1u << qbits.size(), 1u << qbits.size());
	std::complex<double> i(0.0, 1.0);
	double sqrt2 = 1.41421356237;

	if (s == "H" || s == "h") {
		data << 1.0, 1.0, 1.0, -1.0;
		data /= sqrt2;
		return std::make_shared<MatrixGate>(data, qbits, "h");
	} else if (s == "X" || s == "x") {
		data << 0.0, 1.0, 1.0, 0.0;
		return std::make_shared<MatrixGate>(data, qbits, "x");
	} else if (s == "Y" || s == "y") {
		data << 0.0, 1.0*i, -1.0*i, 0.0;
		return std::make_shared<MatrixGate>(data, qbits);
	} else if (s == "Z" || s == "z") {
		data << 1.0, 0.0, 0.0, -1.0;
		return std::make_shared<MatrixGate>(data, qbits);
	} else if (s == "RX" || s == "Rx" || s == "rx") {
		return std::make_shared<RxRotationGate>(qbits);
	} else if (s == "RY" || s == "Ry" || s == "ry") {
		return std::make_shared<RyRotationGate>(qbits);
	} else if (s == "RZ" || s == "Rz" || s == "rz") {
		return std::make_shared<RzRotationGate>(qbits);
//	} else if (s == "U1") {
//		return std::make_shared<SingleQubitParametrizedGate>(qbits);
//	} else if (s == "U2") {
//		return std::make_shared<TwoQubitParametrizedGate>(qbits);
	} else if (s == "CZ" || s == "cz") {
		data << 1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 1.0, 0.0,
				0.0, 0.0, 0.0, -1.0;
		return std::make_shared<MatrixGate>(data, qbits, "cz");
	} else if (s == "CX" || s == "cx") {
		data << 1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 1.0,
				0.0, 0.0, 1.0, 0.0;
		return std::make_shared<MatrixGate>(data, qbits, "cx");
	} else if (s == "CY" || s == "cy") {
		data << 1.0, 0.0, 0.0, 0.0,
				0.0, 1.0, 0.0, 0.0,
				0.0, 0.0, 0.0, -1.0*i,
				0.0, 0.0, 1.0*i, 0.0;
		return std::make_shared<MatrixGate>(data, qbits, "cy");
	} else {
		throw std::invalid_argument("Invalid gate type: " + s);
	}
}
