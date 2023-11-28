#pragma once

#include "CliffordState.hpp"
#include "Tableau.hpp"

template <class TableauType = Tableau>
class QuantumCHPState : public CliffordState {
    private:
        TableauType tableau;

        static uint32_t get_num_qubits(const std::string &s) {
            auto substrings = dataframe::utils::split(s, "\n");
            return substrings.size()/2;
        }
        

    public:
        QuantumCHPState(uint32_t num_qubits, int seed=-1)
         : CliffordState(num_qubits, seed), tableau(TableauType(num_qubits)) {}
        
        QuantumCHPState(const std::string &s) : CliffordState(get_num_qubits(s)) {
            auto substrings = dataframe::utils::split(s, "\n");

            tableau = TableauType(system_size());

            for (uint32_t i = 0; i < substrings.size()-1; i++) {
                substrings[i] = substrings[i].substr(1, substrings[i].size() - 3);
                auto chars = dataframe::utils::split(substrings[i], " | ");
    
                auto row = chars[0];
                bool r = chars[1][0] == '1';

                for (uint32_t j = 0; j < system_size(); j++) {
                    tableau.set_x(i, j, row[j] == '1');
                    tableau.set_z(i, j, row[j + system_size()] == '1');
                }

                tableau.set_r(i, r);
            }
        }

        std::string to_string() const {
            std::string s = "";
            s += "Tableau: \n" + tableau.to_string();
            return s;
        }

        std::string to_string_ops() const {
            std::string s = "";
            s += "Tableau: \n" + tableau.to_string_ops();
            return s;
        }

        virtual void h_gate(uint32_t a) override {
            tableau.h_gate(a);
        }

        virtual void s_gate(uint32_t a) override {
            tableau.s_gate(a);
        }

        virtual void cx_gate(uint32_t a, uint32_t b) override {
            tableau.cx_gate(a, b);
        }

        virtual void cz_gate(uint32_t a, uint32_t b) override {
            tableau.h_gate(b);
            tableau.cx_gate(a, b);
            tableau.h_gate(b);
        }

        virtual double mzr_expectation(uint32_t a) override {
            auto [deterministic, _] = tableau.mzr_deterministic(a);
            if (deterministic)
                return 2*int(mzr(a)) - 1.0;
            else
                return 0.0;
        }

        virtual bool mzr(uint32_t a) override {
            return tableau.mzr(a, rng);
        }

        virtual double sparsity() const override {
            return tableau.sparsity();
        }

        virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override {
            uint32_t system_size = this->system_size();
            uint32_t partition_size = qubits.size();

            // Optimization; if partition size is larger than half the system size, 
            // compute the entropy for the smaller subsystem
            if (partition_size > system_size / 2) {
                std::vector<uint32_t> qubits_complement;
                for (uint32_t q = 0; q < system_size; q++) {
                    if (std::find(qubits.begin(), qubits.end(), q) == qubits.end()) {
                        qubits_complement.push_back(q);
                    }
                }

                return entropy(qubits_complement, index);
            }

            // TODO redo with Tableau?
            std::vector<std::vector<bool>> ttableau(system_size, std::vector<bool>(2*partition_size, false));
            for (uint32_t i = 0; i < system_size; i++) {
                for (uint32_t j = 0; j < partition_size; j++) {
                    ttableau[i][j]                  = tableau.x(i + system_size, qubits[j]);
                    ttableau[i][j + partition_size] = tableau.z(i + system_size, qubits[j]);
                }
            }

            uint32_t pivot_row = 0;
            uint32_t row = 0;
            uint32_t leading = 0;

            for (uint32_t c = 0; c < 2*partition_size; c++) {
                bool found_pivot = false;
                for (uint32_t i = row; i < system_size; i++) {
                    if (ttableau[i][c]) {
                        pivot_row = i;
                        found_pivot = true;
                        break;
                    }
                }

                if (found_pivot) {
                    std::swap(ttableau[row], ttableau[pivot_row]);

                    for (uint32_t i = row + 1; i < system_size; i++) {
                        if (ttableau[i][c]) {
                            for (uint32_t j = 0; j < 2*partition_size; j++) {
                                bool v1 = ttableau[row][j];
                                bool v2 = ttableau[i][j];
                                ttableau[i][j] = v1 ^ v2;
                            }
                        }
                    }

                    leading += 1;
                    row += 1;
                } else {
                    leading += 2;
                    continue;
                }
            }

            uint32_t rank = 0;
            for (uint32_t i = 0; i < system_size; i++) {
                if (std::accumulate(ttableau[i].begin(), ttableau[i].end(), 0)) {
                    rank++;
                }
            }

            int s = rank - partition_size;

            return static_cast<double>(s);
        }
};
