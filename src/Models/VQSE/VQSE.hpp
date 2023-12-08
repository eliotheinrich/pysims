#pragma once

#include <ADAMOptimizer.hpp>
#include <QuantumCircuit.h>
#include <QuantumState.h>

#include <functional>
#include <optional>
#include <stdexcept>

#define VQSE_ADAPTIVE_HAMILTONIAN 0
#define VQSE_LOCAL_HAMILTONIAN 1
#define VQSE_GLOBAL_HAMILTONIAN 2

#define VQSE_EXACT_SAMPLING 0
#define VQSE_SIMULATED_SAMPLING 1

#define VQSE_QUANTUMCIRUIT 0
#define VQSE_DENSITYMATRIX 1
#define VQSE_STATEVECTOR 2

#define DEFAULT_NUM_SHOTS 1024


typedef std::variant<QuantumCircuit, DensityMatrix, Statevector> target_t;

static inline uint32_t to_uint(const std::vector<bool>& vals) {
	uint32_t i = 0;
	for (uint32_t j = 0; j < vals.size(); j++) {
		if (vals[j]) {
			i += 1u << j;
		}
	}

	return i;
}

class VQSE {
	private:
		QuantumCircuit ansatz;
		uint32_t num_qubits;


		uint32_t m;

		uint32_t hamiltonian_type;

		bool sampling_type;
		uint32_t num_shots;

		std::mt19937 rng;

		uint32_t epoch;
		uint32_t update_frequency;
		uint32_t num_iterations;
		ADAMOptimizer optimizer;


		static DensityMatrix make_density_target(const target_t& target) {
			if (target.index() == VQSE_QUANTUMCIRUIT) {
				return DensityMatrix(std::get<QuantumCircuit>(target));
			} else if (target.index() == VQSE_DENSITYMATRIX) {
				return std::get<DensityMatrix>(target);
			} else {
        return DensityMatrix(std::get<Statevector>(target));
      }
		}


		std::map<uint32_t, double> get_outcomes_exact(const QuantumCircuit& circuit, const target_t& target) {
      // Optimization to get exact results
      if (target.index() == VQSE_STATEVECTOR) {
        Statevector state = std::get<Statevector>(target);
        return state.probabilities_map();
      }

			DensityMatrix rho = VQSE::make_density_target(target);
			rho.evolve(circuit);

			return rho.probabilities_map();
		}

		std::map<uint32_t, double> get_outcomes_simulated(const QuantumCircuit& circuit, const target_t& target) {
			std::map<uint32_t, uint32_t> outcome_hist;

      std::vector<double> probabilities;
			if (target.index() == VQSE_QUANTUMCIRUIT) {
        DensityMatrix rho(std::get<QuantumCircuit>(target));
        probabilities = rho.probabilities();
			} else if (target.index() == VQSE_DENSITYMATRIX) {
				DensityMatrix rho = std::get<DensityMatrix>(target);
				rho.evolve(circuit);
			  probabilities = rho.probabilities();
			} else {
				Statevector state = std::get<Statevector>(target);
				state.evolve(circuit);
				probabilities = state.probabilities();
      }

      std::discrete_distribution<uint32_t> dist(probabilities.begin(), probabilities.end());
      for (uint32_t i = 0; i < num_shots; i++) {
        uint32_t outcome = dist(rng);
        if (outcome_hist.count(outcome)) {
          outcome_hist[outcome]++;
        } else {
          outcome_hist.emplace(outcome, 1);
        }
      }

			std::map<uint32_t, double> outcomes;
			for (auto const &[b, c] : outcome_hist) {
				outcomes.emplace(b, float(c)/num_shots);
			}

			return outcomes;
		}

		void update_eigenvalue_estimates(const std::map<uint32_t, double>& outcomes) {
			std::vector<std::pair<uint32_t, double>> pairs;

			for (const auto& pair : outcomes) {
				pairs.push_back(pair);
			}
			
			std::sort(pairs.begin(), pairs.end(), 
				[](const auto& a, const auto& b){ return a.second > b.second; });
			
			for (uint32_t i = 0; i < m; i++) {
				bitstring_estimates[i] = pairs[i].first;
				eigenvalue_estimates[i] = pairs[i].second;
			}
		}

		double cost_function(const std::vector<double>& params, const target_t& target) {
      auto rho = make_density_target(target);
			epoch++;

			QuantumCircuit circuit = ansatz.bind_params(params);

			std::map<uint32_t, double> outcomes;

			if (sampling_type == VQSE_EXACT_SAMPLING) {
				outcomes = get_outcomes_exact(circuit, target);
			} else {
				outcomes = get_outcomes_simulated(circuit, target);
			}

			if (epoch % update_frequency == 0) {
				update_eigenvalue_estimates(outcomes);
			}

			return compute_energy_estimate(outcomes);
		}

		double global_energy(const std::map<uint32_t, double>& outcomes) const {
			double energy = 1.;
			for (uint32_t i = 0; i < m; i++) {
				uint32_t z = bitstring_estimates[i];
				if (outcomes.count(z)) {
					energy -= q[i]*outcomes.at(z);
				}
			}

			return energy;
		}

		double local_energy(const std::map<uint32_t, double>& outcomes) const {
			double energy = 1.;

			for (auto const &[b, p] : outcomes) {
				for (uint32_t j = 0; j < ansatz.num_qubits; j++) {
					energy += r[j]*(2*int((b >> j) & 1) - 1)*p;
				}
			}

			return energy;
		}

		double compute_energy_estimate(const std::map<uint32_t, double>& outcomes) const {
			double t = optimizer.t/double(num_iterations);

			if (hamiltonian_type == VQSE_ADAPTIVE_HAMILTONIAN) {
				return (1 - t)*local_energy(outcomes) + t*global_energy(outcomes);
			} else if (hamiltonian_type == VQSE_LOCAL_HAMILTONIAN) {
				return local_energy(outcomes);
			} else if (hamiltonian_type == VQSE_GLOBAL_HAMILTONIAN) {
				return global_energy(outcomes);
			}
			
			return -1;
		}

	public:
		std::vector<double> get_local_energy_levels(std::optional<uint32_t> energy_levels = std::nullopt) const {
			uint32_t num_energy_levels = energy_levels.value_or(1u << num_qubits);

			uint32_t s = 1u << num_qubits;

			std::vector<double> local_energy_levels;
			for (uint32_t z = 0; z < s; z++) {
				std::map<uint32_t, double> outcomes;
				outcomes[z] = 1.0;
				local_energy_levels.push_back(local_energy(outcomes));
			}

			std::sort(local_energy_levels.begin(), local_energy_levels.end());
			std::vector<double> lowest_energy_levels;
			for (uint32_t i = 0; i < num_energy_levels; i++) {
				lowest_energy_levels.push_back(local_energy_levels[i]);
			}
			return lowest_energy_levels;
		}

		std::vector<double> get_global_energy_levels(std::optional<uint32_t> energy_levels = std::nullopt) const {
			uint32_t num_energy_levels = energy_levels.value_or(1u << num_qubits);

			uint32_t s = 1u << num_qubits;

			std::vector<double> global_energy_levels;
			for (uint32_t z = 0; z < s; z++) {
				std::map<uint32_t, double> outcomes;
				outcomes[z] = 1.0;
				global_energy_levels.push_back(global_energy(outcomes));
			}

			std::sort(global_energy_levels.begin(), global_energy_levels.end());
			std::vector<double> lowest_energy_levels;
			for (uint32_t i = 0; i < num_energy_levels; i++) {
				lowest_energy_levels.push_back(global_energy_levels[i]);
			}
			return lowest_energy_levels;
		}

		std::vector<double> get_energy_levels(std::optional<uint32_t> energy_levels = std::nullopt) const {
			uint32_t num_energy_levels = energy_levels.value_or(1u << num_qubits);

			uint32_t s = 1u << num_qubits;

			std::vector<double> total_energy_levels;
			for (uint32_t z = 0; z < s; z++) {
				std::map<uint32_t, double> outcomes;
				outcomes[z] = 1.0;
				total_energy_levels.push_back(compute_energy_estimate(outcomes));
			}

			std::sort(total_energy_levels.begin(), total_energy_levels.end());
			std::vector<double> lowest_energy_levels;
			for (uint32_t i = 0; i < num_energy_levels; i++) {
				lowest_energy_levels.push_back(total_energy_levels[i]);
			}
			return lowest_energy_levels;
		}

		inline double global_energy_by_bitstring(uint32_t z) const {
			double energy = 1.;
			auto it = std::find(bitstring_estimates.begin(), bitstring_estimates.end(), z);
			if (it != bitstring_estimates.end()) {
				energy -= q[it - bitstring_estimates.begin()];
			}

			return energy;
		}

		inline double local_energy_by_bitstring(uint32_t z) const {
			double energy = 1.;
			for (uint32_t j = 0; j < ansatz.num_qubits; j++) {
				energy += r[j]*(2*int((z >> j) & 1) - 1);
			}
			
			return energy;
		}

		std::vector<double> q;
		std::vector<double> r;
		std::vector<double> eigenvalue_estimates;
		std::vector<uint32_t> bitstring_estimates;
		std::vector<double> params;

		Eigen::MatrixXcd compute_eigenvector_estimates(const std::vector<double>& params) const {
			Eigen::MatrixXcd eigenvectors = Eigen::MatrixXcd::Zero(m, 1u << num_qubits);

			QuantumCircuit circuit = ansatz.bind_params(params).adjoint();

			for (uint32_t i = 0; i < m; i++) {
				Statevector state(num_qubits, bitstring_estimates[i]);
				state.evolve(circuit);
				eigenvectors.row(i) = state.data;
			}

			return eigenvectors;
		}


		VQSE() {}

		VQSE(
			const QuantumCircuit& ansatz, 
			uint32_t m, 
			uint32_t num_iterations, 
			uint32_t hamiltonian_type=VQSE_ADAPTIVE_HAMILTONIAN, 
			uint32_t update_frequency=30,
			bool sampling_type=false,
			uint32_t num_shots=DEFAULT_NUM_SHOTS,
			std::optional<ADAMOptimizer> optimizer = std::nullopt)
		: ansatz(ansatz), m(m), hamiltonian_type(hamiltonian_type), sampling_type(sampling_type), num_shots(num_shots),
		  update_frequency(update_frequency), num_iterations(num_iterations) {
			
			this->optimizer = (optimizer == std::nullopt) ? ADAMOptimizer() : optimizer.value();
			std::random_device rd;
			this->rng = std::mt19937(rd());

			num_qubits = ansatz.num_qubits;

			eigenvalue_estimates = std::vector<double>(m);
			bitstring_estimates = std::vector<uint32_t>(m);
			std::iota(bitstring_estimates.begin(), bitstring_estimates.end(), 0);

			double d = 1./(2.*m);
			r = std::vector<double>(ansatz.num_qubits);
			for (uint32_t i = 0; i < ansatz.num_qubits; i++) {
				r[i] = 1. + double(i)*d;
			}

			q = std::vector<double>(m);

			// Ground state of local Hamiltonian
			double E1 = 1 - 0.5*d*num_qubits*(num_qubits - 1) - num_qubits;

			// Choose q so that local and global Hamiltonians have same energy levels
			q[0] = 1. - E1;
			for (uint32_t i = 1; i < m; i++) {
				q[i] = -1. - E1 - 2*d*(i - 1);
			}
		}

		Eigen::VectorXd true_eigenvalues(const target_t& target) const {
      if (target.index() == VQSE_STATEVECTOR) {
        Eigen::VectorXd val(1); val << 1.0;
        return val;
      }

			DensityMatrix rho = VQSE::make_density_target(target);
			Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> solver;
			solver.compute(rho.data);
			return solver.eigenvalues().tail(m).reverse();
		}

		std::pair<Eigen::VectorXd, Eigen::MatrixXcd> true_eigensystem(const target_t& target) const {
      if (target.index() == VQSE_STATEVECTOR) {
        Eigen::VectorXd val(1); val << 1.0;
        Eigen::MatrixXcd vec = std::get<Statevector>(target).data.transpose();

        return std::make_pair(val, vec);
      }

      // QuantumCircuits and DensityMatrix must both be converted to evolved DensityMatrix and diagonalized

			Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> solver;
			DensityMatrix rho = VQSE::make_density_target(target);
			solver.compute(rho.data);

			Eigen::VectorXd eigenvalues = solver.eigenvalues().tail(m).reverse();
			uint32_t s = (1u << num_qubits);
			Eigen::MatrixXcd eigenvectors = solver.eigenvectors().block(0,s-m,s,m).rowwise().reverse().transpose();
      //Eigen::VectorXcd vec = solver.eigenvectors().block(0,N-1,N,1).rowwise().reverse();

			return std::make_pair(eigenvalues, eigenvectors);
		}

		std::vector<double> fidelity(const target_t& target, const std::vector<double>& params) const {
			auto [true_eigenvalues, true_eigenvectors] = true_eigensystem(target);
			auto estimated_eigenvectors = compute_eigenvector_estimates(params);

			std::vector<double> f;
			for (uint32_t i = 0; i < m; i++) {
				Statevector true_state(true_eigenvectors.row(i));
				Statevector estimated_state(estimated_eigenvectors.row(i));

				f.push_back(std::abs(true_state.inner(estimated_state)));
			}
		
			return f;
		}

		std::pair<double, double> error(const target_t& target) const {
			auto eigenvalues = true_eigenvalues(target);

			double rel_err = 0.;
			double abs_err = 0.;

			for (uint32_t i = 0; i < m; i++) {
				abs_err += std::pow(eigenvalues(i) - eigenvalue_estimates[i], 2);
				rel_err += std::pow((eigenvalues(i) - eigenvalue_estimates[i])/eigenvalues[i], 2);
			}

			return std::make_pair(abs_err, rel_err);
		}

		std::vector<double> optimize(
			const target_t& target, 
			const std::vector<double>& initial_params, 
			std::optional<std::function<void(const std::vector<double>&)>> callback = std::nullopt
		) {
      if (target.index() == VQSE_STATEVECTOR && m != 1) {
        throw std::invalid_argument("Cannot target a Statevector with m != 1.");
      }

    	auto target_cost_function = [this, &target](std::vector<double>& params) { return cost_function(params, target); };
			epoch = 0;

			params = initial_params;
			params = optimizer.minimize(target_cost_function, params, num_iterations, callback);
			return params;
		}
};
