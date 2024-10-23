import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from dataframe import *
from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, h, sre_type="montecarlo", num_runs=1):
    config = {}
    config["circuit_type"] = "half_quantum_ising"
    config["num_runs"] = num_runs

    system_size = list(system_size)
    config["system_size"] = system_size
    config["h"] = h
    config["bond_dimension"] = 25
    config["num_sweeps"] = 10

    config["sre_method"] = sre_type
    config["num_samples"] = 1000
    config["equilibration_timesteps"] = 500

    return config


if __name__ == "__main__":
    L = [8]
    h = np.linspace(0.0, 2.0, 50)
    for Li in L:
        param_matrix = generate_config([Li], h, sre_type=["exhaustive", "exact", "montecarlo"], num_runs=1)
        submit_jobs(f"half_ising_test{Li}", param_bundle=param_matrix, ncores=4, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=True)
