from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np


def checkerboard_ldpc_config(system_size, p4, delete_plaquettes=True, single_site=True, nruns=1):
    config = {"circuit_type": "slanted_checkerboard"}
    
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["p4"] = p4

    config["single_site"] = single_site
    config["delete_plaquettes"] = delete_plaquettes

    config["sample_sym"] = True
    config["sample_leaf_removal"] = False 
    config["sample_rank"] = True
    config["sample_solveable"] = True


    config["max_size"] = 20
    
    return config

system_size = [8, 16, 32]
p = list(np.linspace(0.0, 1.0, 25))
config = checkerboard_ldpc_config(system_size=system_size, p4=p, single_site=[0,1], delete_plaquettes=[0,1], nruns=10)
submit_jobs(config, f"checkboard_test", ncores=48, nodes=5, memory="50gb", time="24:00:00")
