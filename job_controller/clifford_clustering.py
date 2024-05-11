from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def clifford_clustering_(system_size, mzr_prob, num_copies=2, equilibration_timesteps=0, sampling_timesteps=0, measurement_freq=1):
    config = {"circuit_type": "clifford_clustering"}

    config["system_size"] = system_size
    config["mzr_prob"] = mzr_prob
    config["num_copies"] = num_copies

    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = measurement_freq

    return config

config = clifford_clustering_(system_size=[16, 32, 64, 128], mzr_prob=list(np.linspace(0.0, 0.3, 20)), num_copies=40, equilibration_timesteps=500, sampling_timesteps=10)
submit_jobs(config, f"clifford_clustering", ncores=48, memory="10gb", time="4:00:00", cleanup=True, nodes=1)