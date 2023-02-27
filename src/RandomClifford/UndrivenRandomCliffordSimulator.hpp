#ifndef UDRC_SIM_H
#define UDRC_SIM_H

#include "RandomCliffordSimulator.h"

class UndrivenRandomCliffordSimulator : public RandomCliffordSimulator {
	private:
		virtual float mzr_prob(uint i) { return init_mzr_prob; }

	public:
		UndrivenRandomCliffordSimulator(uint system_size, float mzr_prob, uint gate_width, CliffordType clifford_state)
		 : RandomCliffordSimulator(system_size, mzr_prob, gate_width, clifford_state) {}
};

#endif