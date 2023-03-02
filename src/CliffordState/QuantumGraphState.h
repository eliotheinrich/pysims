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



class QuantumGraphState : public CliffordState {
	private:
		static const uint ZGATES[4];
		static const uint CONJUGATION_TABLE[24];
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
		Graph *graph;

        QuantumGraphState(uint num_qubits);
        ~QuantumGraphState() {}

        virtual std::string to_string() const;
        virtual uint system_size() const;

        virtual void h_gate(uint a);
        virtual void s_gate(uint a);

		virtual void x_gate(uint a);
		virtual void y_gate(uint a);
		virtual void z_gate(uint a);

        virtual void cz_gate(uint a, uint b);
        virtual bool mzr(uint a);
		virtual bool mzr_forced(uint a, bool outcome);

        virtual float entropy(std::vector<uint> &qubits) const;


};

#endif