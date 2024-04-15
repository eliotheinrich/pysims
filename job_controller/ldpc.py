from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def ldpc_config(system_size, pr, pb, nruns=1):
    config = {"circuit_type": "ldpc"}
    
    config["model_type"] = 0
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["pr"] = pr
    config["pb"] = pb

    config["sample_leaf_removal"] = True
    config["sample_rank"] = True
    #config["sample_all_locality"] = True
    config["sample_solveable"] = True

    config["max_size"] = 20
    try:
        config["num_leaf_removal_steps"] = max(list(system_size))
    except TypeError:
        config["num_leaf_removal_steps"] = system_size
    
    return config

def regular_ldpc_config(system_size, pr, k, nruns=1):
    config = {"circuit_type": "ldpc"}
    
    config["model_type"] = 1
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["pr"] = pr
    config["k"] = k

    config["sample_leaf_removal"] = True
    config["sample_rank"] = True
    #config["sample_all_locality"] = True
    config["sample_solveable"] = True

    config["max_size"] = 20
    try:
        config["num_leaf_removal_steps"] = max(list(system_size))
    except TypeError:
        config["num_leaf_removal_steps"] = system_size 
    
    return config

# SMALL TEST CASE
#system_size = [8]
#pr = list(np.linspace(0., 1.0, 10))
#pb = list(np.linspace(0., 1.0, 10))
#nruns = 100
#config = ldpc_config(system_size=system_size, pr=pr, pb=pb, nruns=nruns)
#submit_jobs(config, f"small_ldpc_erdos", ncores=1, memory="5gb", time="5:00:00", run_local=True)

system_size = [16, 32, 64, 128]
pr = list(np.linspace(0., 1.0, 20))
pb = list(np.linspace(0., 1.0, 20))
nruns = 50
config = ldpc_config(system_size=system_size, pr=pr, pb=pb, nruns=nruns)
submit_jobs(config, f"ldpc_erdos_del", ncores=8, nodes=10, memory="2gb", time="5:00:00")

system_size = [8, 32, 64, 128]
p = list(np.linspace(0., 1.0, 20))
k = [3, 5]
config = regular_ldpc_config(system_size=system_size, pr=p, k=k, nruns=5)
submit_jobs(config, f"ldpc_regular_del", ncores=8, nodes=10, memory="2gb", time="05:00:00")
