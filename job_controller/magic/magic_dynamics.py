from mps import generate_param_matrix, config_generator
from dataframe import *
from pysims import submit_jobs
import numpy as np

if __name__ == "__main__":
    sre_method = "virtual"
    num_nodes = 3
    L = [32, 48, 64]

    for Li in L:
        param_matrix = generate_param_matrix(
            [Li], beta=[1.2], p=0.5, measurement_type=2, unitary_type=2, bond_dimension=64,
            sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method=sre_method, sre_num_samples=5000,
            sampling_timesteps=200, measurement_freq=3, equilibration_timesteps=0,
            num_runs=180
        )
        submit_jobs(f"magic_dynamics_{Li}", config_generator, param_bundle=param_matrix, ncores=60, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)
