#pragma once

#include <string>
#include <vector>
#include <random>
#include <variant>
#include <unordered_set>
#include <algorithm>

#include <iostream>

#include "QuantumState.h"

#include <unsupported/Eigen/KroneckerProduct>
#include <Eigen/Dense>

namespace tableau_utils {

    struct sgate { uint32_t q; };
    struct sdgate { uint32_t q; };
    struct hgate { uint32_t q;};
    struct cxgate { uint32_t q1; uint32_t q2; };

    typedef std::variant<sgate, sdgate, hgate, cxgate> Gate;
    typedef std::vector<Gate> Circuit;

    template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

}

static tableau_utils::Circuit conjugate_circuit(const tableau_utils::Circuit &circuit) {
    tableau_utils::Circuit ncircuit;
    for (auto const &gate : circuit) {
        std::visit(tableau_utils::overloaded{
            [&ncircuit](tableau_utils::sgate s) { ncircuit.push_back(tableau_utils::sdgate{s.q}); },
            [&ncircuit](tableau_utils::sdgate s) { ncircuit.push_back(tableau_utils::sgate{s.q}); },
            [&ncircuit](auto s) { ncircuit.push_back(s); }
        }, gate);
    }

    std::reverse(ncircuit.begin(), ncircuit.end());

    return ncircuit;
}

template <class T>
static void apply_circuit(const tableau_utils::Circuit &circuit, T &state) {
    for (auto const &gate : circuit) {
        std::visit(tableau_utils::overloaded{
                [&state](tableau_utils::sgate s) {  
                    state.s_gate(s.q); 
                },
                [&state](tableau_utils::sdgate s) { 
                    state.sd_gate(s.q);
                },
                [&state](tableau_utils::hgate s) {  
                    state.h_gate(s.q); 
                },
                [&state](tableau_utils::cxgate s) { 
                    state.cx_gate(s.q1, s.q2); 
                }
        }, gate);
    }
}

template <typename T>
static void remove_even_indices(std::vector<T> &v) {
    uint32_t vlen = v.size();
    for (uint32_t i = 0; i < vlen; i++) {
        uint32_t j = vlen - i - 1;
        if ((j % 2)) v.erase(v.begin() + j);
    }
}

inline static uint32_t get_width(uint32_t N, uint32_t w) {
    return N / w + static_cast<bool>(N % w);
}

class PauliString {
    public:
        uint32_t num_qubits;
        std::vector<bool> bit_string;
        bool phase;

        PauliString()=default;
        PauliString(uint32_t num_qubits) : num_qubits(num_qubits), bit_string(std::vector<bool>(2*num_qubits, false)), phase(false) {}

        static PauliString rand(uint32_t num_qubits, std::minstd_rand *r) {
            PauliString p(num_qubits);

            std::transform(p.bit_string.begin(), p.bit_string.end(), p.bit_string.begin(), [&r](bool) { 
                return (*r)() % 2; 
            });

            p.set_r((*r)() % 2);

            // Need to check that at least one bit is nonzero so that p is not the identity
            for (uint32_t j = 0; j < 2*num_qubits; j++) {
                if (p.bit_string[j]) {
                    return p;
                }
            }

            return PauliString::rand(num_qubits, r);
        }

        static PauliString basis(uint32_t num_qubits, std::string P, uint32_t q, bool r) {
            PauliString p(num_qubits);
            if (P == "X") {
                p.set_x(q, true);
            } else if (P == "Y") {
                p.set_x(q, true);
                p.set_z(q, true);
            } else if (P == "Z") {
                p.set_z(q, true);
            } else {
                std::string error_message = P + " is not a valid basis. Must provide one of X,Y,Z.\n";
                throw std::invalid_argument(error_message);
            }

            p.set_r(r);

            return p;
        }

        static PauliString basis(uint32_t num_qubits, std::string P, uint32_t q) {
            return PauliString::basis(num_qubits, P, q, false);
        }

        PauliString copy() {
            PauliString p(num_qubits);
            std::copy(bit_string.begin(), bit_string.end(), p.bit_string.begin());
            p.set_r(r());
            return p;
        }

        inline bool operator[](size_t i) {
            return bit_string[i];
        }

        inline void set(size_t i, bool b) {
            bit_string[i] = b;
        }

        std::vector<bool>::iterator begin() {
            return bit_string.begin();
        }

        std::vector<bool>::iterator end() {
            return bit_string.end();
        }

        Eigen::Matrix2cd to_matrix(uint32_t i) const {
            std::string s = to_op(i);

            Eigen::Matrix2cd g;
            if (s == "I") {
                g << 1, 0, 0, 1;
            } else if (s == "X") {
                g << 0, 1, 1, 0;
            } else if (s == "Y") {
                g << 0, -1j, 1j, 0;
            } else {
                g << 1, 0, 0, -1;
            }

            return g;
        }

        Eigen::MatrixXcd to_matrix() const {
            Eigen::MatrixXcd g = to_matrix(0);

            for (uint32_t i = 1; i < num_qubits; i++) {
                Eigen::MatrixXcd gi = to_matrix(i);
                Eigen::MatrixXcd g0 = g;
                g = Eigen::kroneckerProduct(gi, g0);
            }
            
            if (phase) {
                g = -g;
            }
        
            return g;
        }

        std::string to_op(uint32_t i) const {
            bool xi = x(i); 
            bool zi = z(i);

            if (xi && zi) {
                return "Y";
            } else if (!xi && zi) {
                return "Z";
            } else if (xi && !zi) {
                return "X";
            } else {
                return "I";
            }
        }

        std::string to_string() const {
            std::string s = "[ ";
            for (uint32_t i = 0; i < 2*num_qubits; i++) {
                s += bit_string[i] ? "1" : "0";
            }
            s += " | ";

            s += phase ? "1 ]" : "0 ]";

            return s;
        }

        std::string to_string_ops() const {
            std::string s = phase ? "-" : "+";
            for (uint32_t i = 0; i < num_qubits; i++) {
                s += to_op(i);
            }
            return s;
        }

        void s_gate(uint32_t a) {
            bool xa = x(a);
            bool za = z(a);
            bool r = phase;

            set_r(r != (xa && za));
            set_z(a, xa != za);
        }

        void h_gate(uint32_t a) {
            bool xa = x(a);
            bool za = z(a);
            bool r = phase;

            set_r(r != (xa && za));
            set_x(a, za);
            set_z(a, xa);
        }

        void cx_gate(uint32_t a, uint32_t b) {
            bool xa = x(a);
            bool za = z(a);
            bool xb = x(b);
            bool zb = z(b);
            bool r = phase;

            set_r(r != ((xa && zb) && ((xb != za) != true)));
            set_x(b, xa != xb);
            set_z(a, za != zb);
        }

        bool commutes_at(PauliString &p, uint32_t i) const {
            if ((x(i) == p.x(i)) && (z(i) == p.z(i))) { // operators are identical
                return true;
            } else if (!x(i) && !z(i)) { // this is identity
                return true;
            } else if (!p.x(i) && !p.z(i)) { // other is identity
                return true;
            } else {
                return false; 
            }
        }

        bool commutes(PauliString &p) const {
            if (num_qubits != p.num_qubits) {
                throw std::invalid_argument("number of p does not have the same number of qubits.");
            }

            uint32_t anticommuting_indices = 0u;
            for (uint32_t i = 0; i < num_qubits; i++) {
                if (!commutes_at(p, i)) {
                    anticommuting_indices++;
                }
            }

            return anticommuting_indices % 2 == 0;
        }

        // Returns the circuit which maps this PauliString onto ZII... if z or XII.. otherwise
        tableau_utils::Circuit reduce(bool z) const;

        // Returns the circuit which maps this PauliString onto p
        tableau_utils::Circuit transform(PauliString const &p) const;


        bool operator==(const PauliString &rhs) const {
            if (num_qubits != rhs.num_qubits) {
                return false;
            }
            
            if (r() != rhs.r()) {
                return false;
            }
            
            for (uint32_t i = 0; i < num_qubits; i++) {
                if (x(i) != rhs.x(i)) {
                    return false;
                }

                if (z(i) != rhs.z(i)) {
                    return false;
                }
            }

            return true;
        }

		bool operator!=(const PauliString &rhs) const { 
            return !(this->operator==(rhs)); 
        }

        inline bool x(uint32_t i) const { 
            return bit_string[i]; 
        }

        inline bool z(uint32_t i) const { 
            return bit_string[i + num_qubits]; 
        }

        inline bool r() const { 
            return phase; 
        }

        inline void set_x(uint32_t i, bool v) { 
            bit_string[i] = v; 
        }

        inline void set_z(uint32_t i, bool v) { 
            bit_string[i + num_qubits] = v; 
        }

        inline void set_r(bool v) { 
            phase = v; 
        }

        void sd_gate(uint32_t a) {
            s_gate(a);
            s_gate(a);
            s_gate(a);
        }
};

class Tableau {
    private:
        uint32_t num_qubits;
        bool track_destabilizers;

    public:
        std::vector<PauliString> rows;

        Tableau()=default;

        Tableau(uint32_t num_qubits)
         : num_qubits(num_qubits), track_destabilizers(true) {
            rows = std::vector<PauliString>(2*num_qubits + 1, PauliString(num_qubits));
            for (uint32_t i = 0; i < num_qubits; i++) {
                rows[i].set_x(i, true);
                rows[i + num_qubits].set_z(i, true);
            }
        }

        Tableau(uint32_t num_qubits, const std::vector<PauliString>& rows)
         : num_qubits(num_qubits), track_destabilizers(false), rows(rows) {}

        uint32_t num_rows() const { 
            if (track_destabilizers) { 
                return rows.size() - 1; 
            } else {
                return rows.size(); 
            }
        }

        Statevector to_statevector() const {
            Eigen::MatrixXcd dm = Eigen::MatrixXcd::Identity(1u << num_qubits, 1u << num_qubits);
            Eigen::MatrixXcd I = Eigen::MatrixXcd::Identity(1u << num_qubits, 1u << num_qubits);
            
            for (uint32_t i = num_qubits; i < 2*num_qubits; i++) {
                PauliString p = rows[i];
                Eigen::MatrixXcd g = p.to_matrix();
                dm = dm*((I + g)/2.0);
            }

            uint32_t N = 1u << num_qubits;
            Eigen::SelfAdjointEigenSolver<Eigen::MatrixXcd> solver(dm);
            Eigen::VectorXcd vec = solver.eigenvectors().block(0,N-1,N,1).rowwise().reverse();
            
            return Statevector(vec);

        }

        bool operator==(Tableau& other) {
            if (num_qubits != other.num_qubits) {
                return false;
            }

            rref();
            other.rref();

            uint32_t r1 = track_destabilizers ? num_qubits : 0;
            for (uint32_t i = r1; i < num_rows(); i++) {
                if (r(i) != other.r(i)) {
                    return false;
                }

                for (uint32_t j = 0; j < num_qubits; j++) {
                    if (z(i, j) != other.z(i, j)) {
                        return false;
                    }

                    if (x(i, j) != other.x(i, j)) {
                        return false;
                    }
                }
            }

            return true;
        }

        // Put tableau into reduced row echelon form
        void rref(bool track_phase=true) {
            uint32_t r1 = track_destabilizers ? num_qubits : 0;
            uint32_t r2 = num_rows();

            uint32_t pivot_row = 0;
            uint32_t row = r1;


            for (uint32_t c = 0; c < 2*num_qubits; c++) {
                bool found_pivot = false;
                for (uint32_t i = row; i < r2; i++) {
                    if (rows[i][c]) {
                        pivot_row = i;
                        found_pivot = true;
                        break;
                    }
                }

                if (found_pivot) {
                    //std::cout << "Pivoting on " << pivot_row << std::endl;
                    //std::cout << "Swapping " << row << " with " << pivot_row << ".\n";
                    std::swap(rows[row], rows[pivot_row]);

                    for (uint32_t i = r1; i < r2; i++) {
                        if (i != row && rows[i][c]) {
                            rowsum(i, row, track_phase);
                        }
                    }

                    row += 1;
                } else {
                    continue;
                }
            }
        }

        uint32_t rank(bool track_phase=true) {
            rref(track_phase);

            uint32_t r = 0;
            for (uint32_t i = 0; i < num_rows(); i++) {
                if (std::accumulate(rows[i].begin(), rows[i].end(), 0)) {
                    r++;
                }
            }

            return r;
        }

        inline void validate_qubit(uint32_t a) const {
            if (!(a >= 0 && a < num_qubits)) {
                std::string error_message = "A gate was applied to qubit " + std::to_string(a) + 
                                            ", which is outside of the allowed range (0, " + std::to_string(num_qubits) + ").";
                throw std::invalid_argument(error_message);
            }
        }

        std::string to_string() const {
            std::string s = "";
            for (uint32_t i = 0; i < num_rows(); i++) {
                s += (i == 0) ? "[" : " ";
                s += rows[i].to_string();
                s += (i == 2*rows.size() - 1) ? "]" : "\n";
            }
            return s;
        }

        std::string to_string_ops() const {
            std::string s = "";
            uint32_t r1 = track_destabilizers ? num_qubits : 0;
            for (uint32_t i = r1; i < num_rows(); i++) {
                s += "[";
                s += rows[i].to_string_ops();
                s += (i == 2*rows.size() - 1) ? "]" : "]\n";
            }
            return s;
        }

        int g(bool x1, bool z1, bool x2, bool z2) {
            if (!x1 && !z1) { 
                return 0; 
            } else if (x1 && z1) {
                if (z2) { 
                    return x2 ? 0 : 1;
                } else { 
                    return x2 ? -1 : 0;
                }
            } else if (x1 && !z1) {
                if (z2) { 
                    return x2 ? 1 : -1;
                } else { 
                    return 0; 
                }
            } else {
                if (x2) { 
                    return z2 ? -1 : 1;
                } else { 
                    return 0; 
                }
            }
        }

        void rowsum(uint32_t h, uint32_t i, bool track_phase=true) {
            //if (!track_destabilizers)
            //    throw std::invalid_argument("Cannot perform rowsum without track_destabilizers.");

            if (track_phase) {
                int s = 0;
                if (r(i)) { 
                    s += 2; 
                }
                if (r(h)) { 
                    s += 2; 
                }

                for (uint32_t j = 0; j < num_qubits; j++)
                    s += Tableau::g(x(i,j), z(i,j), x(h,j), z(h,j));

                if (s % 4 == 0) {
                    set_r(h, false);
                } else if (std::abs(s % 4) == 2) {
                    set_r(h, true);
                }
            }

            for (uint32_t j = 0; j < 2*num_qubits; j++) {
                bool v1 = rows[h][j];
                bool v2 = rows[i][j];
                rows[h].set(j, v1 ^ v2);
            }
        }

        void h_gate(uint32_t a) {
            validate_qubit(a);
            for (uint32_t i = 0; i < num_rows(); i++) {
                rows[i].h_gate(a);
            }
        }

        void s_gate(uint32_t a) {
            validate_qubit(a);
            for (uint32_t i = 0; i < num_rows(); i++) {
                rows[i].s_gate(a);
            }
        }

        void cx_gate(uint32_t a, uint32_t b) {
            validate_qubit(a);
            validate_qubit(b);
            for (uint32_t i = 0; i < num_rows(); i++) {
                rows[i].cx_gate(a, b);
            }
        }

        // Returns a pair containing (1) wether the outcome of a measurement on qubit a is deterministic
        // and (2) the index on which the CHP algorithm performs rowsum if the mzr is random
        std::pair<bool, uint32_t> mzr_deterministic(uint32_t a) {
            if (!track_destabilizers) {
                throw std::invalid_argument("Cannot check mzr_deterministic without track_destabilizers.");
            }

            for (uint32_t p = num_qubits; p < 2*num_qubits; p++) {
                // Suitable p identified; outcome is random
                if (x(p, a)) { 
                    return std::pair(false, p);
                }
            }

            // No p found; outcome is deterministic
            return std::pair(true, 0);
        }

        bool mzr(uint32_t a, std::minstd_rand& rng) {
            validate_qubit(a);
            if (!track_destabilizers) {
                throw std::invalid_argument("Cannot mzr without track_destabilizers.");
            }


            auto [deterministic, p] = mzr_deterministic(a);

            if (!deterministic) {
                bool outcome = rng() % 2;
                for (uint32_t i = 0; i < 2*num_qubits; i++) {
                    if (i != p && x(i, a)) {
                        rowsum(i, p);
                    }
                }

                // TODO check that copy is happening, not passing reference
                std::swap(rows[p - num_qubits], rows[p]);
                rows[p] = PauliString(num_qubits);

                set_r(p, outcome);
                set_z(p, a, true);

                return outcome;
            } else { // deterministic
                rows[2*num_qubits] = PauliString(num_qubits);
                for (uint32_t i = 0; i < num_qubits; i++) {
                    rowsum(2*num_qubits, i + num_qubits);
                }

                return r(2*num_qubits);
            }
        }

        double sparsity() const {
            float nonzero = 0;
            for (uint32_t i = 0; i < num_rows(); i++) {
                for (uint32_t j = 0; j < num_qubits; j++) {
                    nonzero += rows[i].x(j);
                    nonzero += rows[i].z(j);
                }
            }

            return nonzero/(num_rows()*num_qubits*2);
        }


        inline bool x(uint32_t i, uint32_t j) const { 
            return rows[i].x(j); 
        }

        inline bool z(uint32_t i, uint32_t j) const { 
            return rows[i].z(j); 
        }

        inline bool r(uint32_t i) const { 
            return rows[i].r(); 
        }

        inline void set_x(uint32_t i, uint32_t j, bool v) { 
            rows[i].set_x(j, v); 
        }

        inline void set_z(uint32_t i, uint32_t j, bool v) { 
            rows[i].set_z(j, v); 
        }

        inline void set_r(uint32_t i, bool v) { 
            rows[i].set_r(v); 
        }

        void sd_gate(uint32_t a) {
            s_gate(a);
            s_gate(a);
            s_gate(a);
        }

        void x_gate(uint32_t a) {
            h_gate(a);
            z_gate(a);
            h_gate(a);
        }

        void y_gate(uint32_t a) {
            x_gate(a);
            z_gate(a);
        }

        void z_gate(uint32_t a) {
            s_gate(a);
            s_gate(a);
        }
};