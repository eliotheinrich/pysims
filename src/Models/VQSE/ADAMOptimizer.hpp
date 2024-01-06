#pragma once

#include <vector>
#include <cmath>
#include <functional>
#include <optional>
#include <random>

#define PI 3.141592653589793

class ADAMOptimizer {
	private:
		double learning_rate;
		double beta1;
		double beta2;
		double epsilon;
		std::vector<double> m;
		std::vector<double> v;

		std::mt19937 generator;
		std::normal_distribution<double> noise_distribution;

		int gradient_type;
		bool noisy_gradients;
		double gradient_noise;

	public:
		int t;

		ADAMOptimizer(
			std::optional<double> learning_rate = std::nullopt,
			std::optional<double> beta1 = std::nullopt,
			std::optional<double> beta2 = std::nullopt, 
			std::optional<double> epsilon = std::nullopt,
			std::optional<uint32_t> gradient_type = std::nullopt,
			std::optional<bool> noisy_gradients = std::nullopt,
			std::optional<double> gradient_noise = std::nullopt

		) {
			this->learning_rate = learning_rate.value_or(0.001);
			this->beta1 = beta1.value_or(0.9);
			this->beta2 = beta2.value_or(0.999);
			this->epsilon = epsilon.value_or(1e-8);
			
			this->gradient_type = gradient_type.value_or(0u);
			this->noisy_gradients = noisy_gradients.value_or(false);
			this->gradient_noise = gradient_noise.value_or(0.01);

			std::random_device rd;
			this->generator.seed(rd());
			this->noise_distribution = std::normal_distribution<double>(0.0, this->gradient_noise);
		}

		std::vector<double> minimize(
			std::function<double(std::vector<double>&)> cost_func, 
			const std::vector<double>& initial_params, 
			uint32_t num_iterations, 
			std::optional<std::function<void(const std::vector<double>&)>> callback = std::nullopt
		) {
			uint32_t num_params = initial_params.size();
			m.resize(num_params, 0.0);
			v.resize(num_params, 0.0);

			std::vector<double> params = initial_params;

			t = 0;
			for (uint32_t i = 0; i < num_iterations; i++) {
				t++;

				std::vector<double> gradients = compute_gradients(cost_func, params);
				update_params(params, gradients);
				
				if (callback.has_value()) {
					auto callback_func = callback.value();
					callback_func(params);
				}
			}

			return params;
		}

	private:
		std::vector<double> compute_gradients(std::function<double(std::vector<double>&)> cost_func, const std::vector<double>& params) {
			uint32_t num_params = params.size();
			std::vector<double> gradients(num_params, 0.0);

			if (gradient_type == 0) {
				for (uint32_t i = 0; i < num_params; i++) {
					std::vector<double> param_plus = params;
					param_plus[i] += epsilon;
					std::vector<double> param_minus = params;
					param_minus[i] -= epsilon;

					double cost_plus = cost_func(param_plus);
					double cost_minus = cost_func(param_minus);

					double gradient = (cost_plus - cost_minus) / (2 * epsilon);
					gradients[i] = gradient;
				}
			} else if (gradient_type == 1) {
				for (uint32_t i = 0; i < num_params; i++) {
					std::vector<double> param_plus = params;
					param_plus[i] += PI/2;
					std::vector<double> param_minus = params;
					param_minus[i] -= PI/2;

					double cost_plus = cost_func(param_plus);
					double cost_minus = cost_func(param_minus);

					double gradient = (cost_plus - cost_minus)/2.0;
					gradients[i] = gradient;
				}
			}

			if (noisy_gradients) {
				for (uint32_t i = 0; i < num_params; i++) {
					gradients[i] += noise_distribution(generator);
				}
			}

			return gradients;
		}

		void update_params(std::vector<double>& params, const std::vector<double>& gradients) {
			uint32_t num_params = params.size();

			for (uint32_t i = 0; i < num_params; i++) {
				m[i] = beta1 * m[i] + (1 - beta1) * gradients.at(i);
				v[i] = beta2 * v[i] + (1 - beta2) * std::pow(gradients.at(i), 2.0);

				double m_hat = m[i] / (1 - std::pow(beta1, t));
				double v_hat = v[i] / (1 - std::pow(beta2, t));

				params[i] -= learning_rate * m_hat / (std::sqrt(v_hat) + epsilon);
			}
		}
};
