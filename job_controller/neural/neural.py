import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, p, eta, num_runs=1, obc=0, equilibration_timesteps=1000, sampling_timesteps=10000, measurement_freq=1, temporal_avg=True):
    config = {}
    config["circuit_type"] = "lattice_neural"
    config["num_runs"] = num_runs

    config["system_size"] = system_size
    config["p"] = p
    config["eta"] = eta
    config["obc"] = obc
    config["connection_distribution"] = 0

    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = measurement_freq
    config["temporal_avg"] = temporal_avg 
    
    return config
    

if __name__ == "__main__":
    system_sizes = [16, 32, 64]
    eta = list(np.linspace(0.0, 0.25, 50))
    param_bundle = generate_config(system_sizes, 1.0, eta, num_runs=5, obc=1, equilibration_timesteps=0, sampling_timesteps=1000000, measurement_freq=100, temporal_avg=False)
    submit_jobs(f"lattice_neural_obc", param_bundle=param_bundle, ncores=48, memory="10gb", time="6:00:00", nodes=10, cleanup=False, run_local=True)
