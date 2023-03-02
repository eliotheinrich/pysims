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
        QuantumCHPState(uint num_qubits);
        ~QuantumCHPState() {}

        virtual std::string to_string() const;
        virtual uint system_size() const;

        virtual void h_gate(uint a);
        virtual void s_gate(uint a);
        virtual void cx_gate(uint a, uint b);
        virtual void cz_gate(uint a, uint b);
        virtual bool mzr(uint a);
        virtual bool mzr_forced(uint a, bool outcome);

        virtual float entropy(std::vector<uint> &qubits) const;
};

#endif