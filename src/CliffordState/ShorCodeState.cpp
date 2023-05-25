#include "ShorCodeState.h"

ShorCodeState(uint num_qubits, int seed=-1) : num_qubits(num_qubits) {
	state = std::unique_ptr<QuantumCHPState>(new QuantumCHPState(9*num_qubits, seed));
}

std::string to_string() const {
	return encoded_state().to_string();
}

void ShorCodeState::h_gate(uint a) {
	for (uint i = 0; i < 9; i++) {
		state->h_gate(a + i);
		state->x_gate(a + i);
	}
}

void ShorCodeState::s_gate(uint a) {
	for (uint i = 0; i < 9; i++) {

	}
}
void ShorCodeState::cx_gate(uint a, uint b)
void ShorCodeState::cz_gate(uint a, uint b)
bool ShorCodeState::mzr(uint a)

float ShorCodeState::entropy(const std::vector<uint> &qubits) const {
	return encoded_state().entropy(qubits);
}

QuantumCHPState encoded_state() const