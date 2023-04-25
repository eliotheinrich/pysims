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

        virtual std::string to_string() const override;
        virtual uint system_size() const override { return num_qubits; }

        virtual void h_gate(uint a) override;
        virtual void s_gate(uint a) override;
        virtual void cx_gate(uint a, uint b) override;
        virtual void cz_gate(uint a, uint b) override;
        virtual bool mzr(uint a) override;

        virtual float entropy(std::vector<uint> &qubits) const override;
};

#endif