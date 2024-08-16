from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np


def checkerboard_ldpc_config(system_size, p4, delete_plaquettes=True, single_site=True, pbc=True, sample_perp=True, nruns=1):
    config = {"circuit_type": "slanted_checkerboard"}
    
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["p4"] = p4

    config["single_site"] = single_site
    config["delete_plaquettes"] = delete_plaquettes
    config["pbc"] = pbc
    config["sample_perp"] = sample_perp

    config["sample_sym"] = True
    config["sample_leaf_removal"] = False 
    config["sample_rank"] = True
    config["sample_solveable"] = True

    config["sample_all_locality"] = True

    config["max_size"] = 20
    
    return config

system_size = [8, 16, 32]
p = list(np.linspace(0.7, 1.0, 25))
config = checkerboard_ldpc_config(system_size=system_size, p4=p, single_site=1, delete_plaquettes=1, pbc=[0, 1], sample_perp=[0, 1], nruns=500)
submit_jobs(config, f"checkerboard_locality2", ncores=48, nodes=5, memory="50gb", time="24:00:00", run_local=False)
