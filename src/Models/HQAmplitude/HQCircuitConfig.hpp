#pragma once

#include <Frame.h>
#include <BinaryPolynomial.h>

#include <random>
#include <unordered_set>
#include <memory>

static BinaryPolynomial cx_on_poly(const BinaryPolynomial& poly, size_t i, size_t j) {
  std::vector<BinaryPolynomialTerm> terms;
  for (auto const& term : poly.terms) {
    if (term.contains_ind(j)) {
      BinaryPolynomialTerm t1(term);
      BinaryPolynomialTerm t2(term);
      size_t k = std::distance(term.inds.begin(), std::find(term.inds.begin(), term.inds.end(), j));
      t2.inds[k] = i;
      terms.push_back(t1.reduce());
      terms.push_back(t2.reduce());
    } else {
      terms.push_back(term);
    }
  }

  return BinaryPolynomial(terms, poly.n);
}

class HQCircuitConfig {
  public:
    HQCircuitConfig(dataframe::ExperimentParams& params) {
      int seed = dataframe::utils::get<int>(params, "seed", 0);
      if (seed == 0) {
        thread_local std::random_device random_device;
        rng.seed(random_device());
      } else {
        rng.seed(seed);
      }

      k = dataframe::utils::get<int>(params, "k");
      m = std::pow(2, k);
      num_qubits = 3*m;

      p1 = dataframe::utils::get<double>(params, "p1", 0.5);
      p2 = dataframe::utils::get<double>(params, "p2", 0.5);
      p3 = dataframe::utils::get<double>(params, "p3", 0.5);

      calculation_type = dataframe::utils::get<int>(params, "calculation_type", 0);
      num_samples = dataframe::utils::get<int>(params, "num_samples", 0);
      num_polynomials = dataframe::utils::get<int>(params, "num_polynomials", 1);
    }

    double calculate_amplitude(const BinaryPolynomial& poly) const {
      if (calculation_type == 0) {
        // Explicit sum over every element
        uint64_t s = 1u << poly.n;
        double I = 0.0;
        uint32_t positive = 0;
        for (uint64_t z = 0; z < s; z++) {
          Bitstring bits(z, poly.n);
          if (poly.evaluate(bits)) {
            positive++;
          }
          I += std::pow(-1.0, poly.evaluate(bits));
        }

        return I/s;
      } else if (calculation_type == 1) {
        // Use linear algebra technique in IBM paper
        BinaryMatrix gamma(m, m);
        for (size_t i = 0; i < m; i++) {
          for (size_t j = 0; j < m; j++) {
            size_t ib = 3*i + 1;
            size_t ig = 3*j + 2;
            std::vector<size_t> inds{ib, ig};
            if (poly.contains_term(inds)) {
              gamma.set(i, j, 1);
            }
          }
        }

        std::vector<bool> delta_g(m);
        std::vector<bool> delta_b(m);

        for (size_t i = 0; i < m; i++) {
          std::vector<size_t> ind(1);

          size_t ib = 3*i + 1;
          ind[0] = ib;
          if (poly.contains_term(ind)) {
            delta_b[i] = 1;
          }

          size_t ig = 3*i + 2;
          ind[0] = ig;
          if (poly.contains_term(ind)) {
            delta_g[i] = 1;
          }
        }

        if (gamma.in_col_space(delta_g) && gamma.in_row_space(delta_b)) {
          std::vector<bool> solution = gamma.solve_linear_system(delta_g);
          bool r = false;
          for (size_t i = 0; i < m; i++) {
            r ^= delta_b[i] & solution[i];
          }

          uint32_t rank = gamma.rank();
          int f0 = poly.evaluate(Bitstring(poly.n));
          double sign = std::pow(-1.0, double(r) + f0);
          return sign/(1u << rank);
        } else {
          return 0.0;
        }
      } else {
        throw std::invalid_argument("Invalid calculation type.");
      }
    }

    BinaryPolynomial generate_hq_polynomial() {
      BinaryPolynomial poly(num_qubits);

      std::unordered_set<size_t> s1;
      for (size_t q = 0; q < num_qubits; q++) {
        Bitstring bits(q, num_qubits);
        if (bits.hamming_weight() % 2 == 0) {
          s1.insert(q);
        }
      }

      for (size_t i = 0; i < k + 1; i++) {
        // Apply diagonal layer
        for (size_t c = 0; c < m; c++) {
          std::vector<size_t> qubits{3*c, 3*c + 1, 3*c + 2};
          for (size_t q = 0; q < 3; q++) {
            if (randf() < p1) {
              std::vector<size_t> inds{qubits[q]};
              poly.add_term(inds);
            }
          }

          for (size_t q1 = 0; q1 < 3; q1++) {
            for (size_t q2 = q1+1; q2 < 3; q2++) {
              if (randf() < p2) {
                std::vector<size_t> inds{qubits[q1], qubits[q2]};
                poly.add_term(inds);
              }
            }
          }

          for (size_t q1 = 0; q1 < 3; q1++) {
            for (size_t q2 = q1+1; q2 < 3; q2++) {
              for (size_t q3 = q2+1; q3 < 3; q3++) {
                if (randf() < p3) {
                  std::vector<size_t> inds{qubits[q1], qubits[q2], qubits[q3]};
                  poly.add_term(inds);
                }
              }
            }
          }
        }

        if (i == k) {
          break;
        }

        // Apply CX layer
        std::vector<std::pair<size_t, size_t>> partners;
        std::unordered_set<size_t> accounted;
        for (size_t n1 = 0; n1 < m; n1++) {
          size_t n2 = n1 ^ (1u << i);
          if (!accounted.contains(n1) && !accounted.contains(n2)) {
            size_t q1, q2;
            if (s1.contains(n1)) {
              q1 = n1;
              q2 = n2;
            } else {
              q1 = n2;
              q2 = n1;
            }

            partners.push_back(std::make_pair(q1, q2));
            accounted.insert(n1);
            accounted.insert(n2);
          }
        }

        for (auto const& [q1, q2] : partners) {
          for (size_t j = 0; j < 3; j++) {
            poly = cx_on_poly(poly, 3*q1 + j, 3*q2 + j);
          }
        }
      }

      return poly;
    }

    dataframe::DataSlide compute(uint32_t num_threads) {
      std::vector<double> all_amplitudes;
      uint32_t n = 0;
      for (uint32_t k = 0; k < num_polynomials; k++) {
        BinaryPolynomial poly = generate_hq_polynomial();

        std::vector<size_t> inds(m);
        for (size_t i = 0; i < m; i++) {
          inds[i] = 3*i;
        }

        std::vector<double> amplitudes;
        if (num_samples == 0) {
          // Exact calculation (every bitstring)
          uint64_t s = 1u << m;
          amplitudes = std::vector<double>(s);
          for (uint64_t z = 0; z < s; z++) {
            Bitstring bits(z, m);
            BinaryPolynomial poly_z = poly.partial_evaluate(bits, inds);
            amplitudes[z] = calculate_amplitude(poly_z);
          }
        } else {
          // Monte-Carlo calculation
          amplitudes = std::vector<double>(num_samples);
          for (uint32_t i = 0; i < num_samples; i++) {
            Bitstring bits = Bitstring::random(m, rng);
            BinaryPolynomial poly_z = poly.partial_evaluate(bits, inds);
            amplitudes[i] = calculate_amplitude(poly_z);
          }
        }

        all_amplitudes.resize(all_amplitudes.size() + amplitudes.size(), 0);
        for (size_t i = 0; i < amplitudes.size(); i++) {
          all_amplitudes[n] = amplitudes[i];
          n++;
        }
      }

      dataframe::DataSlide slide;
      slide.add_data("amplitudes");
      for (auto const& a : all_amplitudes) {
        slide.push_samples_to_data("amplitudes", a);
      }

      return slide;
    }

  private:
    size_t k;
    size_t m;
    size_t num_qubits;

    double p1;
    double p2;
    double p3;

    uint32_t calculation_type;
    uint32_t num_samples;
    uint32_t num_polynomials;

    std::minstd_rand rng;

    int rand() {
      return rng();
    }

    double randf() {
      return double(rng())/double(RAND_MAX);
    }

};
