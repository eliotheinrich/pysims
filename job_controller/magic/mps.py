import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from dataframe import *
from job_controller import submit_jobs
import numpy as np

def generate_config(
        system_size, beta, p, measurement_type=1, unitary_type=1,
        subsystem_fraction=8, sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method="montecarlo", sre_num_samples=5000,
        num_runs=1, sampling_timesteps=50, measurement_freq=5, equilibration_timesteps=30
    ):

    config = {}
    config["config_type"] = "mps_simulator"
    config["num_runs"] = num_runs

    system_size = list(system_size)
    zparams = [{"system_size": L, "magic_mutual_information_subsystem_size": L//subsystem_fraction} for L in system_size]
    config["zparams2"] = zparams
    config["bond_dimension"] = 128
    config["beta"] = beta
    config["p"] = p

    config["unitary_type"] = unitary_type
    config["measurement_type"] = measurement_type

    config["sample_magic_mutual_information"] = sample_mi 
    config["sample_bipartite_magic_mutual_information"] = sample_bipartite_mi
    config["sample_stabilizer_renyi_entropy"] = sample_sre
    config["sre_method"] = sre_method
    config["sre_mc_equilibration_timesteps"] = 500
    config["sre_num_samples"] = sre_num_samples

    config["sample_surface"] = True

    config["temporal_avg"] = False
    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = measurement_freq

    return config

if __name__ == "__main__":
    sample_sre = True
    sample_mi = True
    sample_bipartite_mi = False
    sre_method = ["montecarlo"]
    num_runs = 4
    num_nodes = 20

    sampling_timesteps = 50
    measurement_freq = 5
    equilibration_timesteps = 50

    L = [8, 16, 32, 64]
    param_matrix = generate_config(
        L, beta=1.2, p=0.5, measurement_type=1, unitary_type=2,
        sample_sre=False, sample_mi=False, sample_bipartite_mi=True, sre_method="virtual", sre_num_samples=5000,
        sampling_timesteps=sampling_timesteps, measurement_freq=measurement_freq, equilibration_timesteps=equilibration_timesteps,
        num_runs=num_runs
    )

    submit_jobs(f"xxz_z2_bipartite_magic", param_bundle=param_matrix, ncores=64, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)

    sample_sre = False
    sample_mi = False
    sample_bipartite_mi = False

    L = [128]
    beta = np.linspace(0.8, 1.5, 20)
    #p = np.linspace(0.3, 0.7, 20)
    p = 0.5
    param_matrix = generate_config(
        L, beta=beta, p=p, measurement_type=1, unitary_type=2,
        sample_sre=sample_sre, sample_mi=sample_mi, sample_bipartite_mi=sample_bipartite_mi,
        sampling_timesteps=500, measurement_freq=1, equilibration_timesteps=50,
        num_runs=40
    )
    #submit_jobs(f"xxz_z2_surface_beta", param_bundle=param_matrix, ncores=64, memory="10gb", time="36:00:00", nodes=3, cleanup=False, run_local=False)
