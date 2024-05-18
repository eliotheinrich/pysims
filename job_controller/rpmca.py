from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def rpmca3_config(system_size, p, nruns=1):
    config = {"circuit_type": "rpmca"}
    
    config["model_type"] = 0
    config["num_runs"] = nruns
    
    config["model_type"] = 0
    config["system_size"] = system_size
    config["p"] = p

    config["sample_generators"] = True
    
    return config
    
def rpmca5_config(system_size, p, nruns=1):
    config = {"circuit_type": "rpmca"}
    
    config["model_type"] = 0
    config["num_runs"] = nruns
    
    config["model_type"] = 1
    config["system_size"] = system_size
    config["p"] = p

    config["sample_generators"] = True
    
    return config
    

system_size = [256]
p = list(np.linspace(0.6, 0.9, 30))
config = rpmca3_config(system_size=system_size, p=p, nruns=1)
#submit_jobs(config, f"rpmca3", ncores=4, nodes=1, memory="50gb", time="24:00:00", run_local=True)

config = rpmca5_config(system_size=system_size, p=p, nruns=1)
submit_jobs(config, f"rpmca5", ncores=30, nodes=1, memory="50gb", time="24:00:00", run_local=False, cleanup=False)