from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def xorsat_config(num_variables, k, num_rows, nruns=1):
    config = {"circuit_type": "xorsat"}
    
    # VQSE parameters
    config["num_runs"] = nruns
    
    config["num_variables"] = num_variables
    config["k"] = k
    config["num_rows"] = num_rows

    config["sample_leaf_removal"] = True
    config["sample_rank"] = True
    config["sample_solveable"] = True
    config["sample_all_locality"] = True

    config["include_isolated_in_core"] = [0, 1]

    config["max_size"] = 20
    try:
        config["num_leaf_removal_steps"] = max(list(num_variables))
    except TypeError:
        config["num_leaf_removal_steps"] = num_variables
    
    return config

for i in [32, 64, 128, 256, 512]:
    #config = xorsat_config(num_variables=i, k=3, num_rows=list(range(1, int(1.5*i), max(i//20, 1))), nruns=1000)
    config = xorsat_config(num_variables=i, k=3, num_rows=list(range(1, int(10*i), max(i//20, 1))), nruns=1000)
    submit_jobs(config, f"xorsat3_{i}", ncores=64, memory="5gb", time="2:00:00", cleanup=False)
