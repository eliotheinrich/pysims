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

    config["max_size"] = 20
    try:
        config["num_leaf_removal_steps"] = max(list(num_variables))
    except TypeError:
        config["num_leaf_removal_steps"] = num_variables
    
    return config

def ldpc_config(system_size, impurity, boundary_conditions=0, nruns=1):
    config = {"circuit_type": "lattice_ldpc"}
    
    # VQSE parameters
    config["num_runs"] = nruns

    zparams = [{"system_size": L, "spacing": L} for L in system_size]
    config["zparams"] = zparams
    config["impurity"] = impurity
    config["boundary_conditions"] = boundary_conditions

    config["sample_leaf_removal"] = True
    config["sample_rank"] = True
    config["sample_solveable"] = True
    
    return config


for i in [32, 64, 128, 256]:
    #config = xorsat_config(num_variables=i, k=3, num_rows=list(range(1, int(1.5*i), max(i//20, 1))), nruns=1000)
    config = xorsat_config(num_variables=i, k=3, num_rows=list(range(1, int(1.5*i), max(i//20, 1))), nruns=1000)
    submit_jobs(config, f"xorsat2_{i}", ncores=4, memory="5gb", time="1:00:00", run_local=True, cleanup=False)

impurity = list(np.linspace(0.0, 1.0, 50))
#config = ldpc_config(system_size=[5, 10, 20, 40], impurity=impurity, boundary_conditions=1, nruns=100)
#submit_jobs(config, "lattice_ldpc", ncores=4, memory="1gb", time="4:00:00", run_local=True)