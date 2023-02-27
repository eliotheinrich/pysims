#include "QuantumCHPState.h"
#include <random>
#include <assert.h>
#include <algorithm>

QuantumCHPState::QuantumCHPState(uint num_qubits) : num_qubits(num_qubits), 
                                                    tableau(Tableau(num_qubits)) {}


std::string QuantumCHPState::to_string() const {
    std::string s = "";
    s += "Tableau: \n" + tableau.to_string();
    return s;
}

uint QuantumCHPState::system_size() const {
    return num_qubits;
}

void QuantumCHPState::h_gate(uint a) {
//std::cout << "h " << a << std::endl;
    tableau.h_gate(a);
}

void QuantumCHPState::s_gate(uint a) {
//std::cout << "s " << a << std::endl;
    tableau.s_gate(a);
}

void QuantumCHPState::cx_gate(uint a, uint b) {
//std::cout << "cx " << a << " " << b << std::endl;
    tableau.cx_gate(a, b);
}

void QuantumCHPState::cz_gate(uint a, uint b) {
    tableau.h_gate(b);
    tableau.cx_gate(a, b);
    tableau.h_gate(b);
}

bool QuantumCHPState::mzr(uint a) {
    bool outcome = rand() % 2 == 0;
    tableau.mzr(a, outcome);
    return outcome;
}

float QuantumCHPState::entropy(std::vector<uint> &qubits) const {
    uint system_size = this->system_size();
    uint partition_size = qubits.size();

    // TODO redo with Tableau?
    std::vector<std::vector<bool>> ttableau(system_size, std::vector<bool>(2*partition_size, false));
    for (uint i = 0; i < system_size; i++) {
        for (uint j = 0; j < partition_size; j++) {
            ttableau[i][j]                  = tableau.x(i + system_size, qubits[j]);
            ttableau[i][j + partition_size] = tableau.z(i + system_size, qubits[j]);
        }
    }

    uint pivot_row = 0;
    uint row = 0;
    uint leading = 0;

    for (uint c = 0; c < 2*partition_size; c++) {
        bool found_pivot = false;
        for (uint i = row; i < system_size; i++) {
            if (ttableau[i][c]) {
                pivot_row = i;
                found_pivot = true;
                break;
            }
        }

        if (found_pivot) {
            std::swap(ttableau[row], ttableau[pivot_row]);

            for (uint i = row + 1; i < system_size; i++) {
                if (ttableau[i][c]) {
                    for (uint j = 0; j < 2*partition_size; j++) {
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

    uint rank = 0;
    for (uint i = 0; i < system_size; i++) {
        if (std::accumulate(ttableau[i].begin(), ttableau[i].end(), 0)) {
            rank++;
        }
    }
    float s = rank - partition_size;
if (s > 1000) std::cout << "rank = " << rank << ", partition_size = " << partition_size << std::endl;
    return rank - partition_size;
}
