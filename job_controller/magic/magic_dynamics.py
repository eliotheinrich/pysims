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

    L = [16, 32, 64, 128, 256, 512]
    param_matrix = generate_param_matrix(
        L, beta=[1.2], p=0.5, measurement_type=1, unitary_type=2,
        sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method="virtual", sre_num_samples=5000,
        sampling_timesteps=10, measurement_freq=1, equilibration_timesteps=0,
        num_runs=50
    )

    submit_jobs(f"magic_dynamics_L", param_bundle=param_matrix, ncores=60, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)

    num_nodes = 10

    L = [16]
    param_matrix = generate_param_matrix(
        L, beta=[0.3, 0.4, 0.5, 0.6], p=0.5, measurement_type=0, unitary_type=0,
        sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method="virtual", sre_num_samples=5000,
        sampling_timesteps=10, measurement_freq=1, equilibration_timesteps=0,
        num_runs=50
    )

    submit_jobs(f"magic_dynamics_haar", param_bundle=param_matrix, ncores=60, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)

    L = [16]
    param_matrix = generate_param_matrix(
        L, beta=np.arange(0.1, 1.3, 0.1), p=0.5, measurement_type=1, unitary_type=2,
        sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method="virtual", sre_num_samples=5000,
        sampling_timesteps=5, measurement_freq=1, equilibration_timesteps=0,
        num_runs=50
    )

    submit_jobs(f"magic_dynamics_beta", param_bundle=param_matrix, ncores=60, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)
