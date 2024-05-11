from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def xorsat_config(system_size, num_rows, nruns=1):
    config = {"circuit_type": "graph_xorsat"}
    
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["k"] = 2
    config["num_rows"] = num_rows

    config["sample_surface"] = True
    config["sample_surface_avg"] = True
    config["sample_rugosity"] = False 
    config["sample_roughness"] = False
    config["sample_avalanche_sizes"] = False
    config["sample_structure_function"] = False
    
    return config

for i in [16, 32, 64, 128]:
    config = xorsat_config(system_size=i, num_rows=list(range(1, int(20*i), max(i//20, 1))), nruns=1000)
    submit_jobs(config, f"graph_xorsat_{i}", ncores=64, memory="10gb", time="2:00:00", cleanup=False)
