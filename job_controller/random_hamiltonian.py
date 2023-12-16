import numpy as np

from job_controller import config_to_string, submit_jobs, save_config
from pysims.pysimulators import *


def rh_config(num_qubits, dt, mu, sigma, sampling_timesteps, entropy_sampling=True, nruns=1000):
    config = {"circuit_type": "random_hamiltonian"}
    
    config["num_runs"] = nruns
    config["sample_probabilities"] = False
    config["sample_bitstring_distribution"] = False
    config["max_prob"] = 0.05

    config["dt"] = dt
    config["mu"] = mu
    config["sigma"] = sigma
    
    config["sample_all_partition_sizes"] = entropy_sampling
    config["spatial_avg"] = True
    config["sample_fixed_mutual_information"] = True

    zparams_mi = []
    if isinstance(num_qubits, int):
        num_qubits = [num_qubits]
    for n in num_qubits:
        zparams_mi.append({
            "system_size": n,
            "x1": 0,
            "x2": 1,
            "x3": n//2,
            "x4": n//2 + 1    
        })
    config["zparams_mi"] = zparams_mi

    config["equilibration_timesteps"] = 0
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = 1
    config["temporal_avg"] = False
    
    return config_to_string(config)

config = rh_config(8, dt=0.1, mu=0.0, sigma=1.0, sampling_timesteps=120, nruns=1250)
submit_jobs(config, "random_hamiltonian", ncores=4, nodes=1, memory="250gb", time="24:00:00", run_local=True)