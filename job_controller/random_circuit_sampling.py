import numpy as np

from job_controller import config_to_string, submit_jobs, save_config
from pysims.pysimulators import *


def rcs_config(num_qubits, evolution_type, mzr_probs, sampling_timesteps, entropy_sampling=False, nruns=1000):
    config = {"circuit_type": "random_circuit_sampling"}
    
    config["num_runs"] = nruns
    config["sample_probabilities"] = True
    config["sample_bitstring_distribution"] = True
    config["max_prob"] = 0.05
    
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

    config["mzr_prob"] = mzr_probs
    config["evolution_type"] = evolution_type
    
    config["equilibration_timesteps"] = 0
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = 1
    config["temporal_avg"] = False
    
    return config_to_string(config)

mzr_probs = list(np.linspace(0.0, 0.5, 20))
config = rcs_config([12], 2, mzr_probs, sampling_timesteps=120, nruns=1250)
submit_jobs(config, "rcs_dist48_12", ncores=48, nodes=4, memory="250gb", time="24:00:00")

#mzr_probs = [0.0]
#config = rcs_config([14], 1, mzr_probs, sampling_timesteps=28, nruns=1000)
#submit_jobs(config, "rcs_brickwork_clean", ncores=48, memory="250gb", time="24:00:00")

mzr_probs = np.linspace(0.0, 0.5, 20)
config = rcs_config(14, 1, mzr_probs, sampling_timesteps=20, entropy_sampling=True)
#submit_jobs(config, "rcs_ds", ncores=48, memory="250gb", time="24:00:00")
