#ifndef QCHP_H
#define QCHP_H

#include <vector>

using uint = unsigned int;

class PauliString {
    public:
        uint num_qubits;
        std::vector<bool> bit_string;
        bool phase;

        PauliString(uint num_qubits);
        static PauliString rand(uint num_quibts);

        std::string to_op(uint i) const;
        std::string to_string(bool to_ops) const;

        bool x(uint i) const { return bit_string[i]; }
        bool z(uint i) const { return bit_string[i + num_qubits]; };
        bool r() const { return phase; };

        void set_x(uint i, bool v) { bit_string[i] = v; }
        void set_z(uint i, bool v) { bit_string[i + num_qubits] = v; }
        void set_r(bool v) { phase = v; }

        bool commutes_at(PauliString &p, uint i) const;
        bool commutes(PauliString &p) const;
};

class Tableau {
    private:
        uint num_qubits;
        std::vector<PauliString> rows;
        bool track_destabilizers;
        bool print_ops;

    public:
        Tableau(uint num_qubits);
        uint num_rows() const { if (track_destabilizers) { return rows.size() - 1; } else { return rows.size(); }};
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