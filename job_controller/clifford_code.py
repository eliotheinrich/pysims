from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def clifford_code_config(system_size, mzr_prob, nruns=1):
    config = {"circuit_type": "clifford_code"}
    
    # VQSE parameters
    config["num_runs"] = nruns
    
    config["system_size"] = system_size 
    config["mzr_prob"] = mzr_prob

    config["equilibration_timesteps"] = 200
    config["sampling_timesteps"] = 200

    config["temporal_avg"] = True
    config["equilibration_timesteps"] = 200
    config["measurement_freq"] = 2
    config["sampling_timesteps"] = 200
    
    return config

config = clifford_code_config(system_size=[16, 32, 64], mzr_prob=list(np.linspace(0.0, 1.0, 20)), nruns=10)
submit_jobs(config, f"clifford_code", ncores=4, memory="5gb", time="1:00:00", run_local=True, cleanup=True)