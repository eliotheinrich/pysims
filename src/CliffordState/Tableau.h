#ifndef TABLEAU_H
#define TABLEAU_H

#include <string>
#include <vector>
#include <random>
#include <variant>
#include <algorithm>

struct sgate { uint q; };
struct sdgate { uint q; };
struct hgate { uint q;};
struct cxgate { uint q1; uint q2; };

typedef std::variant<sgate, sdgate, hgate, cxgate> Gate;
typedef std::vector<Gate> Circuit;


template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

static Circuit conjugate_circuit(const Circuit &circuit) {
    Circuit ncircuit;
    for (auto const &gate : circuit) {
        std::visit(overloaded{
            [&ncircuit](sgate s) { ncircuit.push_back(sdgate{s.q}); },
            [&ncircuit](sdgate s) { ncircuit.push_back(sgate{s.q}); },
            [&ncircuit](auto s) { ncircuit.push_back(s); }
        }, gate);
    }

    std::reverse(ncircuit.begin(), ncircuit.end());

    return ncircuit;
}

template <class T>
static void apply_circuit(const Circuit &circuit, T &state) {
    for (auto const &gate : circuit) {
        std::visit(overloaded{
                [&state](sgate s) {  state.s_gate(s.q); },
                [&state](sdgate s) { state.sd_gate(s.q); },
                [&state](hgate s) {  state.h_gate(s.q); },
                [&state](cxgate s) { state.cx_gate(s.q1, s.q2); }
        }, gate);
    }
}

template <typename T>
static void remove_even_indices(std::vector<T> &v) {
    uint vlen = v.size();
    for (uint i = 0; i < vlen; i++) {
        uint j = vlen - i - 1;
        if ((j % 2)) v.erase(v.begin() + j);
    }
}


class PauliString {
    public:
        uint num_qubits;
        std::vector<bool> bit_string;
        bool phase;

        PauliString(uint num_qubits);

        static PauliString rand(uint num_qubits, std::minstd_rand *r);
        static PauliString basis(uint num_qubits, std::string P, uint q, bool r);
        static PauliString basis(uint num_qubits, std::string P, uint q) {
            return PauliString::basis(num_qubits, P, q, false);
        }
		PauliString copy();

        std::string to_op(uint i) const;
        std::string to_string(bool to_ops) const;

        bool x(uint i) const { return bit_string[i]; }
        bool z(uint i) const { return bit_string[i + num_qubits]; }
        bool r() const { return phase; }

        void set_x(uint i, bool v) { bit_string[i] = v; }
        void set_z(uint i, bool v) { bit_string[i + num_qubits] = v; }
        void set_r(bool v) { phase = v; }

        void s_gate(uint a);
        void sd_gate(uint a) {
            s_gate(a);
            s_gate(a);
            s_gate(a);
        }
        void h_gate(uint a);
        void cx_gate(uint a, uint b);

        bool commutes_at(PauliString &p, uint i) const;
        bool commutes(PauliString &p) const;

        // Returns the circuit which maps this PauliString onto ZII... if z or XII.. otherwise
        Circuit reduce(bool z) const;

        // Returns the circuit which maps this PauliString onto p
        Circuit transform(PauliString const &p) const;

		bool operator==(const PauliString &rhs) const;
		bool operator!=(const PauliString &rhs) const { return !(this->operator==(rhs)); }
};

class Tableau {
    private:
        uint num_qubits;
        bool track_destabilizers;
        bool print_ops;

    public:
        std::vector<PauliString> rows;

        Tableau(uint num_qubits);
        Tableau(uint num_qubits, std::vector<PauliString> rows);

        uint num_rows() const { if (track_destabilizers) { return rows.size() - 1; } else { return rows.size(); }}
        std::string to_string() const;

        bool x(uint i, uint j) const { return rows[i].x(j); }
        bool z(uint i, uint j) const { return rows[i].z(j); }
        bool r(uint i) const { return rows[i].r(); }

        void set_x(uint i, uint j, bool v) { rows[i].set_x(j, v); }
        void set_z(uint i, uint j, bool v) { rows[i].set_z(j, v); }
        void set_r(uint i, bool v) { rows[i].set_r(v); }

        static int g(bool x1, bool z1, bool x2, bool z2);

        void rowsum(uint h, uint i);

        void h_gate(uint a);
        void s_gate(uint a);
        void sd_gate(uint a) {
            s_gate(a);
            s_gate(a);
            s_gate(a);
        }

        void x_gate(uint a) {
            h_gate(a);
            z_gate(a);
            h_gate(a);
        }
        void y_gate(uint a) {
            x_gate(a);
            z_gate(a);
        }
        void z_gate(uint a) {
            s_gate(a);
            s_gate(a);
        }

        void cx_gate(uint a, uint b);


        std::pair<bool, uint> mzr_deterministic(uint a);
        bool mzr(uint a, bool outcome);
};

#endif