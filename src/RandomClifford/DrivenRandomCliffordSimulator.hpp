#ifndef DRC_SIM_H
#define DRC_SIM_H

#include "RandomCliffordSimulator.h"

class DrivenRandomCliffordSimulator : public RandomCliffordSimulator {
	private:
		// TODO add drive
		virtual float mzr_prob(uint i) { return init_mzr_prob; } //std::exp(-entropy(std::vector<uint>{i})); }

	public:
		DrivenRandomCliffordSimulator(uint system_size, float mzr_prob, uint gate_width, CliffordType clifford_state)
		 : RandomCliffordSimulator(system_size, mzr_prob, gate_width, clifford_state) {}
};

#endif