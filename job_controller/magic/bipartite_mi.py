import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
sys.path.insert(1, os.getcwd())

from mps import generate_config
from dataframe import *
from job_controller import submit_jobs
import numpy as np

if __name__ == "__main__":
    num_nodes = 1

    L = [8, 16, 32]
    param_matrix = generate_config(
        L, beta=1.2, p=0.5, measurement_type=1, unitary_type=2,
        sample_sre=False, sample_mi=False, sample_bipartite_mi=True, sre_method="montecarlo", sre_num_samples=5000,
        sampling_timesteps=50, measurement_freq=5, equilibration_timesteps=50,
        num_runs=20
    )

    submit_jobs(f"bipartite_magic_mc_32", param_bundle=param_matrix, ncores=64, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)

    L = [64]
    param_matrix = generate_config(
        L, beta=1.2, p=0.5, measurement_type=1, unitary_type=2,
        sample_sre=False, sample_mi=False, sample_bipartite_mi=True, sre_method="montecarlo", sre_num_samples=5000,
        sampling_timesteps=50, measurement_freq=5, equilibration_timesteps=50,
        num_runs=60
    )

    #submit_jobs(f"bipartite_magic_mc_64", param_bundle=param_matrix, ncores=64, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)
