#ifndef FF_CONFIG_H
#define FF_CONFIG_H
#include <DataFrame.hpp>
#include "FreeFermionSimulator.h"

#define DEFAULT_NUM_RUNS 1u
#define DEFAULT_EQUILIBRATION_STEPS 0u
#define DEFAULT_TIMESTEPS 0u
#define DEFAULT_MEASUREMENT_FREQ 1u
#define DEFAULT_SPACING 1u

class FreeFermionConfig : Config {
    private:
        uint partition_size;

        uint equilibration_steps;
        uint timesteps;
        uint measurement_freq;

        uint spacing;
        
        float p1;
        float p2;
        float beta;
        float filling_fraction;

        FreeFermionSimulator simulator;

    public:
        uint system_size;
        static std::vector<FreeFermionConfig> load_json(std::string filename);

        FreeFermionConfig();
        FreeFermionConfig(std::map<std::string, int> iparams, std::map<std::string, float> fparams);

        virtual std::map<std::string, int> get_iparams() const;
        virtual std::map<std::string, float> get_fparams() const;
        virtual void compute(DataSlide* slide);

};

#endif