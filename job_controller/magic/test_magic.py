import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))
sys.path.insert(1, os.getcwd())

from mps import generate_param_matrix
from dataframe import *
from job_controller import submit_jobs
import numpy as np

if __name__ == "__main__":
    num_nodes = 10

    L = [8]
    param_matrix = generate_param_matrix(
        L, beta=1.2, p=np.linspace(0.0, 1.0, 25), measurement_type=1, unitary_type=2,
        sample_sre=True, sample_mi=True, subsystem_fraction=2, sample_bipartite_mi=True, sre_method=["montecarlo", "exhaustive", "exact", "virtual"], sre_num_samples=5000,
        sampling_timesteps=50, measurement_freq=5, equilibration_timesteps=50,
        num_runs=5
    )

    submit_jobs(f"test_magic2", param_bundle=param_matrix, ncores=30, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)

    param_matrix = generate_param_matrix(
        L, beta=1.2, p=np.linspace(0.0, 1.0, 3), measurement_type=1, unitary_type=2,
        sample_sre=True, sample_mi=True, subsystem_fraction=2, sample_bipartite_mi=False, sre_method=["montecarlo"], sre_num_samples=5,
        sampling_timesteps=3, measurement_freq=1, equilibration_timesteps=5,
        num_runs=1
    )

    #submit_jobs(f"test_magic_tiny", param_bundle=param_matrix, ncores=1, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=True)
