#ifndef CLIFFORDSIM_H
#define CLIFFORDSIM_H

#include "Tableau.h"
#include "Entropy.hpp"
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


template <typename T>
static void remove_even_indices(std::vector<T> &v) {
    uint vlen = v.size();
    for (uint i = 0; i < vlen; i++) {
        uint j = vlen - i - 1;
        if ((j % 2)) v.erase(v.begin() + j);
    }
}

struct sgate { uint q; };
struct sdgate { uint q; };
struct hgate { uint q;};
struct cxgate {
    uint q1;
    uint q2;
};


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


class CliffordState: public Entropy {
    private:
        std::minstd_rand rng;

        // Returns the circuit which maps a PauliString to Z1 if z, otherwise to X1
        static Circuit reduce(const PauliString &p, bool z = true) {
            uint num_qubits = p.num_qubits;

            Tableau tableau = Tableau(num_qubits, std::vector<PauliString>{p});

            Circuit circuit;

            if (z) {
                tableau.h_gate(0);
                circuit.push_back(hgate{0});
            }

            for (uint i = 0; i < num_qubits; i++) {
                if (tableau.z(0, i)) {
                    if (tableau.x(0, i)) {
                        tableau.s_gate(i);
                        circuit.push_back(sgate{i});
                    } else {
                        tableau.h_gate(i);
                        circuit.push_back(hgate{i});
                    }
                }
            }

            // Step two
            std::vector<uint> nonzero_idx;
            for (uint i = 0; i < num_qubits; i++) {
                if (tableau.x(0, i)) {
                    nonzero_idx.push_back(i);
                }
            }
            while (nonzero_idx.size() > 1) {
                for (uint j = 0; j < nonzero_idx.size()/2; j++) {
                    uint q1 = nonzero_idx[2*j];
                    uint q2 = nonzero_idx[2*j+1];
                    tableau.cx_gate(q1, q2);
                    circuit.push_back(cxgate{q1, q2});
                }

                remove_even_indices(nonzero_idx);
            }

            // Step three
            uint ql = nonzero_idx[0];
            if (ql != 0) {
                for (uint i = 0; i < num_qubits; i++) {
                    if (tableau.x(0, i)) {
                        tableau.cx_gate(0, ql);
                        tableau.cx_gate(ql, 0);
                        tableau.cx_gate(0, ql);

                        circuit.push_back(cxgate{0, ql});
                        circuit.push_back(cxgate{ql, 0});
                        circuit.push_back(cxgate{0, ql});

                        break;
                    }
                }
            }

            if (tableau.r(0)) {
                tableau.y_gate(0);
                circuit.push_back(sgate{0});
                circuit.push_back(sgate{0});
                circuit.push_back(hgate{0});
                circuit.push_back(sgate{0});
                circuit.push_back(sgate{0});
                circuit.push_back(hgate{0});
            }

            if (z) {
                tableau.h_gate(0);
                circuit.push_back(hgate{0});
            }

            return circuit;
        }

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
            while (p1.commutes(p2)) {
                p2 = PauliString::rand(num_qubits, &rng);
            }

            Circuit c1 = reduce(p1, true);

            apply_circuit(c1, p2);
            auto qubit_visitor = overloaded{
                [&qubits](sgate s) -> Gate { return sgate{qubits[s.q]}; },
                [&qubits](sdgate s) -> Gate { return sdgate{qubits[s.q]}; },
                [&qubits](hgate s) -> Gate { return hgate{qubits[s.q]}; },
                [&qubits](cxgate s) -> Gate { return cxgate{qubits[s.q1], qubits[s.q2]}; }
            };

            for (auto &gate : c1)
                gate = std::visit(qubit_visitor, gate);

            apply_circuit(c1, *this);
            
            PauliString z1_p(num_qubits);
            z1_p.set_z(0, true);
            PauliString z1_m(num_qubits);
            z1_m.set_z(0, true);
            z1_m.set_r(true);

            if (p2 != z1_p && p2 != z1_m) {
                Circuit c2 = reduce(p2, false);
                for (auto &gate : c2)
                    gate = std::visit(qubit_visitor, gate);

                apply_circuit(c2, *this);
            }

//            Tableau tableau = Tableau(num_qubits, std::vector<PauliString>{p1, p2});
//
//            // Step one
//            for (uint i = 0; i < num_qubits; i++) {
//                if (tableau.z(0, i)) {
//                    if (tableau.x(0, i)) {
//                        tableau.s_gate(i);
//                        s_gate(qubits[i]);
//                    } else {
//                        tableau.h_gate(i);
//                        h_gate(qubits[i]);
//                    }
//                }
//            }
//
//            // Step two
//            std::vector<uint> nonzero_idx;
//            for (uint i = 0; i < num_qubits; i++) {
//                if (tableau.x(0, i)) {
//                    nonzero_idx.push_back(i);
//                }
//            }
//            while (nonzero_idx.size() > 1) {
//                for (uint j = 0; j < nonzero_idx.size()/2; j++) {
//                    uint q1 = nonzero_idx[2*j];
//                    uint q2 = nonzero_idx[2*j+1];
//                    tableau.cx_gate(q1, q2);
//                    cx_gate(qubits[q1], qubits[q2]);
//                }
//
//                remove_even_indices(nonzero_idx);
//            }
//
//            // Step three
//            uint ql = nonzero_idx[0];
//            if (ql != 0) {
//                for (uint i = 0; i < num_qubits; i++) {
//                    if (tableau.x(0, i)) {
//                        tableau.cx_gate(0, ql);
//                        tableau.cx_gate(ql, 0);
//                        tableau.cx_gate(0, ql);
//
//                        cx_gate(qubits[0], qubits[ql]);
//                        cx_gate(qubits[ql], qubits[0]);
//                        cx_gate(qubits[0], qubits[ql]);
//
//                        break;
//                    }
//                }
//            }
//
//            // Step four
//            PauliString z1_p(num_qubits);
//            z1_p.set_z(0, true);
//            PauliString z1_m(num_qubits);
//            z1_m.set_z(0, true);
//            z1_m.set_r(true);
//
//            if (tableau.rows[1] != z1_p && tableau.rows[1] != z1_m) {
//                tableau.h_gate(0);
//                h_gate(qubits[0]);
//
//                // Repeat step one
//                for (uint i = 0; i < num_qubits; i++) {
//                    if (tableau.z(1, i)) {
//                        if (tableau.x(1, i)) {
//                            tableau.s_gate(i);
//                            s_gate(qubits[i]);
//                        } else {
//                            tableau.h_gate(i);
//                            h_gate(qubits[i]);
//                        }
//                    }
//                }
//
//                // ... and step two
//                nonzero_idx.clear();
//                for (uint i = 0; i < num_qubits; i++) {
//                    if (tableau.x(1, i)) {
//                        nonzero_idx.push_back(i);
//                    }
//                }
//
//                while (nonzero_idx.size() > 1) {
//                    for (uint j = 0; j < nonzero_idx.size()/2; j++) {
//                        uint q1 = nonzero_idx[2*j];
//                        uint q2 = nonzero_idx[2*j+1];
//                        tableau.cx_gate(q1, q2);
//                        cx_gate(qubits[q1], qubits[q2]);
//                    }
//
//                    remove_even_indices(nonzero_idx);
//                }
//
//                tableau.h_gate(0);
//                h_gate(qubits[0]);
//            }
//
//            // Step five: correct phase
//            if (tableau.r(0) && tableau.r(1)) {
//                y_gate(qubits[0]);
//            } else if (!tableau.r(0) && tableau.r(1)) {
//                x_gate(qubits[0]);
//            } else {
//                z_gate(qubits[0]);
//            }
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

};

#endif