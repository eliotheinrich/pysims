#ifndef FF_CONFIG_H
#define FF_CONFIG_H
#include <DataFrame.hpp>
#include "FreeFermionSimulator.h"

#define DEFAULT_NUM_RUNS 1u
#define DEFAULT_EQUILIBRATION_STEPS 0u
#define DEFAULT_TIMESTEPS 0u
#define DEFAULT_MEASUREMENT_FREQ 1u
#define DEFAULT_SPACING 1u
#define DEFAULT_TEMPORAL_AVG true

class FreeFermionConfig : public TimeConfig, public Entropy {
    private:
        float p1;
        float p2;
        float beta;
        float filling_fraction;

        FreeFermionSimulator *simulator;

		virtual void init_state();
		virtual std::map<std::string, Sample> take_samples();

    public:
        static std::vector<FreeFermionConfig*> load_json(std::string filename);

        FreeFermionConfig(Params &p);

        virtual float entropy(std::vector<uint> &sites) { return simulator->entropy(sites); }
        virtual void init_state();
        virtual void timesteps(uint num_steps);

};

#endif