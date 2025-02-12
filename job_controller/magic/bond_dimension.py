import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
sys.path.insert(1, os.getcwd())

from mps import generate_param_matrix
from dataframe import *
from job_controller import submit_jobs
import numpy as np

if __name__ == "__main__":
    num_nodes = 1

    L = [128]
    param_matrix = generate_param_matrix(
        L, bond_dimension=8, beta=[0.0], p=0.5, measurement_type=0, unitary_type=0,
        sample_sre=False, sample_mi=False, subsystem_fraction=2, sample_bipartite_mi=False, sre_method="virtual", sre_num_samples=5000,
        sampling_timesteps=10, measurement_freq=1, equilibration_timesteps=0,
        num_runs=50
    )

    submit_jobs(f"bond_dimension", param_bundle=param_matrix, ncores=60, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)

