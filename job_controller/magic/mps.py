import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from dataframe import *
from job_controller import submit_jobs
import numpy as np

def generate_param_matrix(
        system_size, beta, p, measurement_type=1, unitary_type=1,
        subsystem_fraction=8, sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method="montecarlo", sre_num_samples=5000,
        num_runs=1, sampling_timesteps=50, measurement_freq=5, equilibration_timesteps=30,
        **kwargs
    ):

    config = {}
    config["config_type"] = "mps_simulator"
    config["num_runs"] = num_runs

    system_size = list(system_size)
    config["system_size"] = ZippedParams([{"system_size": L, "magic_mutual_information_subsystem_size": L//subsystem_fraction} for L in system_size])
    config["bond_dimension"] = 128
    config["beta"] = beta
    config["p"] = p

    config["unitary_type"] = unitary_type
    config["measurement_type"] = measurement_type

    config["sample_magic_mutual_information"] = sample_mi 
    config["sample_bipartite_magic_mutual_information"] = sample_bipartite_mi
    config["sample_stabilizer_renyi_entropy"] = sample_sre
    config["stabilizer_renyi_indices"] = "1,2"
    config["sre_method"] = sre_method
    config["sre_mc_equilibration_timesteps"] = 500
    config["sre_num_samples"] = sre_num_samples

    config["sample_surface"] = True

    config["temporal_avg"] = False
    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = measurement_freq

    for key, val in kwargs.items():
        config[key] = val

    return config
