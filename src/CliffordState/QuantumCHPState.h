#ifndef QCHP_H
#define QCHP_H

#include <vector>
#include <string>
#include "Simulator.hpp"
#include "CliffordState.hpp"
#include "Tableau.h"

class QuantumCHPState : public CliffordState {
    private:
        uint num_qubits;
        Tableau tableau;

    public:
        QuantumCHPState(uint num_qubits, int seed=-1);

        virtual std::string to_string() const;
        virtual uint system_size() const { return num_qubits; }

        virtual void h_gate(uint a);
        virtual void s_gate(uint a);
        virtual void cx_gate(uint a, uint b);
        virtual void cz_gate(uint a, uint b);
        virtual bool mzr(uint a);

        virtual float entropy(std::vector<uint> &qubits) const;
};

#endif