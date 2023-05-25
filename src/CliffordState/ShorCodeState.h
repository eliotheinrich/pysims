#ifndef SHOR_CODE
#define SHOR_CODE

class ShorCodeState : public CliffordState {
	private:
		uint num_qubits;
		std::unique_ptr<QuantumCHPState> state;

	public:
		ShorCodeState(uint num_qubits, int seed=-1);

		virtual std::string to_string() const override;
		virtual uint system_size() const override { return num_qubits; }

		// Physical gates corresponding to noise
		// These are the errors which an error-correcting round will correct
		void px_gate(uint a) { state->x_gate(a); }
		void py_gate(uint a) { state->y_gate(a); }
		void pz_gate(uint a) { state->z_gate(a); }
		void ph_gate(uint a) { state->h_gate(a); }
		void ps_gate(uint a) { state->s_gate(a); }
		bool pmzr(uint a) { return state->mzr(a); }

		// Logical gates
		virtual void h_gate(uint a) override;
		virtual void s_gate(uint a) override;
		virtual void cx_gate(uint a, uint b) override;
		virtual void cz_gate(uint a, uint b) override;
		virtual bool mzr(uint a) override;

		virtual float entropy(const std::vector<uint> &qubits) const override;

		QuantumCHPState encoded_state() const;
};

#endif