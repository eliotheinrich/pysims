import numpy as np

from job_controller import config_to_string, submit_jobs, save_config
from pysims.pysimulators import *


def rcs_config(num_qubits, evolution_type, mzr_probs):
    config = {"circuit_type": "random_circuit_sampling"}
    
    config["num_runs"] = 1
    config["system_size"] = num_qubits
    config["sample_probabilities"] = True
    config["sample_bitstring_distribution"] = True
    config["max_prob"] = 0.2

    config["mzr_prob"] = mzr_probs
    config["evolution_type"] = evolution_type
    
    config["equilibration_timesteps"] = 20
    config["sampling_timesteps"] = 1
    config["temporal_avg"] = True
    
    return config_to_string(config)

mzr_probs = list(np.linspace(0.0, 1.0, 40))
config = rcs_config([8], [1], mzr_probs)
#save_config(config, "../configs/rcs_brickwork_haar.json")
submit_jobs(config, "rcs_brickwork_haar", ncores=4, memory="10gb", time="1:00:00", run_local=True, cleanup=False)

