#ifndef UDRC_SIM_H
#define UDRC_SIM_H

#include "RandomCliffordSimulator.h"

class UndrivenRandomCliffordSimulator : public RandomCliffordSimulator {
	private:
		virtual float mzr_prob(uint i) { return init_mzr_prob; }

	public:
		UndrivenRandomCliffordSimulator(Params &params) : RandomCliffordSimulator(Params &params) {}
};

#endif