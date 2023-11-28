#pragma once

#include <vector>
#include "Graph.h"
#include "CliffordState.hpp"
#include "QuantumCHPState.hpp"

#define IDGATE     0
#define XGATE      1
#define YGATE      2
#define ZGATE      3
#define HGATE      12
#define SGATE      20
#define SDGATE     23
#define SQRTXGATE  17
#define SQRTXDGATE 16
#define SQRTYGATE  15
#define SQRTYDGATE 13
#define SQRTZGATE  20
#define SQRTZDGATE 23


// A simulator for quantum clifford states represented with graphs, as outlined in
// https://arxiv.org/abs/quant-ph/0504117
class QuantumGraphState : public CliffordState {
	private:
		static const uint32_t ZGATES[4];
		static const uint32_t CONJUGATION_TABLE[24];
		static const uint32_t HERMITIAN_CONJUGATE_TABLE[24];
		static const uint32_t CLIFFORD_DECOMPS[24][5];
		static const uint32_t CLIFFORD_PRODUCTS[24][24];
		static const uint32_t CZ_LOOKUP[24][24][2][3];

		uint32_t num_qubits;

		void apply_gater(uint32_t a, uint gate_id);
		void apply_gatel(uint32_t a, uint gate_id);
		void local_complement(uint32_t a);
		void remove_vop(uint32_t a, uint b);
		bool isolated(uint32_t a, uint b);

		void mxr_graph(uint32_t a, bool outcome);
		void myr_graph(uint32_t a, bool outcome);


	public:
		Graph graph;

        QuantumGraphState(uint32_t num_qubits, int seed=-1);
        QuantumGraphState(Graph &graph, int seed=-1);

		QuantumCHPState to_chp() const;

        virtual std::string to_string() const override;

        virtual void h_gate(uint32_t a) override;
        virtual void s_gate(uint32_t a) override;

		virtual void x_gate(uint32_t a) override;
		virtual void y_gate(uint32_t a) override;
		virtual void z_gate(uint32_t a) override;

        virtual void cz_gate(uint32_t a, uint b) override;
		virtual double mzr_expectation(uint32_t a) override;
        virtual bool mzr(uint32_t a) override;
		
		void mzr_graph(uint32_t a, bool outcome);

		void toggle_edge_gate(uint32_t a, uint b);

        virtual double entropy(const std::vector<uint32_t> &qubits, uint32_t index) const override;

		virtual double sparsity() const override;
};