#ifndef CLIFFORDSIM_H
#define CLIFFORDSIM_H

#include "Tableau.h"
#include <random>
#include <deque>
#include <algorithm>
#include <assert.h>
#include <variant>

enum CliffordType { CHP, GraphSim };

static inline CliffordType parse_clifford_type(std::string s) {
    if (s == "chp") return CliffordType::CHP;
    else if (s == "graph") return CliffordType::GraphSim;
    else {
        assert(false);
        return CliffordType::CHP;
    }
}

class CliffordState {
    private:
        std::minstd_rand rng;

        // Returns the circuit which maps a PauliString to Z1 if z, otherwise to X1
        void single_qubit_random_clifford(uint a, uint r) {
            // r == 0 is identity, so do nothing in thise case
            if (r == 1) {
                x_gate(a);
            } else if (r == 2) {
                y_gate(a);
            } else if (r == 3) {
                z_gate(a);
            } else if (r == 4) {
                h_gate(a);
                s_gate(a);
                h_gate(a);
                s_gate(a);
            } else if (r == 5) {
                h_gate(a);
                s_gate(a);
                h_gate(a);
                s_gate(a);
                x_gate(a);
            } else if (r == 6) {
                h_gate(a);
                s_gate(a);
                h_gate(a);
                s_gate(a);
                y_gate(a);
            } else if (r == 7) {
                h_gate(a);
                s_gate(a);
                h_gate(a);
                s_gate(a);
                z_gate(a);
            } else if (r == 8) {
                h_gate(a);
                s_gate(a);
            } else if (r == 9) {
                h_gate(a);
                s_gate(a);
                x_gate(a);
            } else if (r == 10) {
                h_gate(a);
                s_gate(a);
                y_gate(a);
            } else if (r == 11) {
                h_gate(a);
                s_gate(a);
                z_gate(a);
            } else if (r == 12) {
                h_gate(a);
            } else if (r == 13) {
                h_gate(a);
                x_gate(a);
            } else if (r == 14) {
                h_gate(a);
                y_gate(a);
            } else if (r == 15) {
                h_gate(a);
                z_gate(a);
            } else if (r == 16) {
                s_gate(a);
                h_gate(a);
                s_gate(a);
            } else if (r == 17) {
                s_gate(a);
                h_gate(a);
                s_gate(a);
                x_gate(a);
            } else if (r == 18) {
                s_gate(a);
                h_gate(a);
                s_gate(a);
                y_gate(a);
            } else if (r == 19) {
                s_gate(a);
                h_gate(a);
                s_gate(a);
                z_gate(a);
            } else if (r == 20) {
                s_gate(a);
            } else if (r == 21) {
                s_gate(a);
                x_gate(a);
            } else if (r == 22) {
                s_gate(a);
                y_gate(a);
            } else if (r == 23) {
                s_gate(a);
                z_gate(a);
            }
        }

        // Performs an iteration of the random clifford algorithm outlined in https://arxiv.org/pdf/2008.06011.pdf
        void random_clifford_iteration(std::deque<uint> &qubits) {
            uint num_qubits = qubits.size();

            // If only acting on one qubit, can easily lookup from a table
            if (num_qubits == 1) {
                single_qubit_random_clifford(qubits[0], rand() % 24);
                return;
			}

            PauliString p1 = PauliString::rand(num_qubits, &rng);
            PauliString p2 = PauliString::rand(num_qubits, &rng);
            while (p1.commutes(p2))
                p2 = PauliString::rand(num_qubits, &rng);

            Circuit c1 = p1.reduce(false);

            apply_circuit(c1, p2);

            auto qubit_map_visitor = overloaded{
                [&qubits](sgate s) -> Gate { return sgate{qubits[s.q]}; },
                [&qubits](sdgate s) -> Gate { return sdgate{qubits[s.q]}; },
                [&qubits](hgate s) -> Gate { return hgate{qubits[s.q]}; },
                [&qubits](cxgate s) -> Gate { return cxgate{qubits[s.q1], qubits[s.q2]}; }
            };

            for (auto &gate : c1)
                gate = std::visit(qubit_map_visitor, gate);

            apply_circuit(c1, *this);

            PauliString z1p = PauliString::basis(num_qubits, "Z", 0, false);
            PauliString z1m = PauliString::basis(num_qubits, "Z", 0, true);

            if (p2 != z1p && p2 != z1m) {
                Circuit c2 = p2.reduce(true);

                for (auto &gate : c2)
                    gate = std::visit(qubit_map_visitor, gate);

                apply_circuit(c2, *this);
            }
        }

    public:
        CliffordState(int seed=-1) {
            if (seed == -1) rng = std::minstd_rand(std::rand());
            else rng = std::minstd_rand(seed);
        }

        virtual ~CliffordState() {}

        virtual uint system_size() const=0;

        uint rand() {
            return this->rng();
        }

        float randf() {
            return float(rand())/float(RAND_MAX);
        }

        virtual void h_gate(uint a)=0;
        virtual void s_gate(uint a)=0;

        virtual void sd_gate(uint a) {
            s_gate(a);
            s_gate(a);
            s_gate(a);
        }

        virtual void x_gate(uint a) {
            h_gate(a);
            z_gate(a);
            h_gate(a);
        }
        virtual void y_gate(uint a) {
            x_gate(a);
            z_gate(a);
        }
        virtual void z_gate(uint a) {
            s_gate(a);
            s_gate(a);
        }

        virtual void cz_gate(uint a, uint b)=0;
        virtual void cx_gate(uint a, uint b) {
            h_gate(b);
            cz_gate(a, b);
            h_gate(b);
        }
        virtual void cy_gate(uint a, uint b) {
            s_gate(b);
            h_gate(b);
            cz_gate(a, b);
            h_gate(b);
            s_gate(b);
            s_gate(b);
            s_gate(b);
        }

        virtual void T4_gate(uint a, uint b, uint c, uint d) {
            cx_gate(a, d);
            cx_gate(b, d);
            cx_gate(c, d);

            cx_gate(d, a);
            cx_gate(d, b);
            cx_gate(d, c);

            cx_gate(a, d);
            cx_gate(b, d);
            cx_gate(c, d);
        }

        virtual bool mzr(uint a)=0;

        virtual bool mxr(uint a) {
            h_gate(a);
            bool outcome = mzr(a);
            h_gate(a);
            return outcome;
        }
        virtual bool myr(uint a) {
            s_gate(a);
            h_gate(a);
            bool outcome = mzr(a);
            h_gate(a);
            s_gate(a);
            s_gate(a);
            s_gate(a);
            return outcome;
        }

        void random_clifford(std::vector<uint> &qubits) {
            for (auto q : qubits) assert(q < system_size());
            uint num_qubits = qubits.size();
            std::deque<uint> dqubits(num_qubits);
            std::copy(qubits.begin(), qubits.end(), dqubits.begin());
            for (uint i = 0; i < num_qubits; i++) {
                random_clifford_iteration(dqubits);
                dqubits.pop_front();
            }
        }

        virtual std::string to_string() const { return ""; };
        virtual float entropy(const std::vector<uint> &qubits) const=0;

};

#endif