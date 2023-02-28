#ifndef DRC_SIM_H
#define DRC_SIM_H

#include "RandomCliffordSimulator.h"

class DrivenRandomCliffordSimulator : public RandomCliffordSimulator {
	private:
		// TODO add drive
		virtual float mzr_prob(uint i) { return init_mzr_prob; } //std::exp(-entropy(std::vector<uint>{i})); }

	public:
		DrivenRandomCliffordSimulator(Params &params) : RandomCliffordSimulator(Params &params) {}
};

#endif