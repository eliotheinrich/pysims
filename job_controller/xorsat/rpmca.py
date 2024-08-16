import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def rpmca3_config(system_size, p, nruns=1):
    config = {"circuit_type": "rpmca"}
    
    config["num_runs"] = nruns
    
    config["model_type"] = 0
    config["system_size"] = system_size
    config["p"] = p

    config["sample_generators"] = True
    
    return config
    
def rpmca5_config(system_size, p, nruns=1):
    config = {"circuit_type": "rpmca"}
    
    config["num_runs"] = nruns
    
    config["model_type"] = 1
    config["system_size"] = system_size
    config["p"] = p

    config["sample_generators"] = True
    
    return config

def rpmca4_config(system_size, p, nruns=1):
    config = {}
    config["circuit_type"] = "rpmca"

    config["model_type"] = 2
    config["num_runs"] = nruns

    config["system_size"] = system_size
    config["p"] = p

    config["sample_generators"] = False
    config["sample_sparsity"] = True

    return config

system_size = [256]
p = list(np.linspace(0.0, 1.0, 40))
config = rpmca3_config(system_size=system_size, p=p, nruns=1)
#submit_jobs(config, f"rpmca3", ncores=4, nodes=1, memory="50gb", time="24:00:00", run_local=True)

system_size = [32, 64, 128]
p = list(np.linspace(0.0, 1.0, 40))
config = rpmca4_config(system_size=system_size, p=p, nruns=100)
submit_jobs(f"rpmca4", param_bundle=config, ncores=64, nodes=1, memory="50gb", time="24:00:00", run_local=False, cleanup=False)
