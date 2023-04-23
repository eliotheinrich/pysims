#ifndef QGRAPH_STATE
#define QGRAPH_STATE

#include <vector>
#include "Graph.h"
#include "CliffordState.hpp"

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
		static const uint ZGATES[4];
		static const uint CONJUGATION_TABLE[24];
		static const uint HERMITIAN_CONJUGATE_TABLE[24];
		static const uint CLIFFORD_DECOMPS[24][5];
		static const uint CLIFFORD_PRODUCTS[24][24];
		static const uint CZ_LOOKUP[24][24][2][3];

		uint num_qubits;

		void apply_gater(uint a, uint gate_id);
		void apply_gatel(uint a, uint gate_id);
		void local_complement(uint a);
		void remove_vop(uint a, uint b);
		bool isolated(uint a, uint b);

		void mxr_graph(uint a, bool outcome);
		void myr_graph(uint a, bool outcome);
		void mzr_graph(uint a, bool outcome);


	public:
		Graph graph;

        QuantumGraphState(uint num_qubits, int seed=-1);

        virtual std::string to_string() const;
        virtual uint system_size() const { return num_qubits; }

        virtual void h_gate(uint a);
        virtual void s_gate(uint a);

		virtual void x_gate(uint a);
		virtual void y_gate(uint a);
		virtual void z_gate(uint a);

        virtual void cz_gate(uint a, uint b);
        virtual bool mzr(uint a);

		void toggle_edge_gate(uint a, uint b);

        virtual float entropy(std::vector<uint> &qubits) const;


};

#endif