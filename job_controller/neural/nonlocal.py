import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, eta, alpha, J=1.0, fully_connected=0, num_runs=1, equilibration_timesteps=1000, sampling_timesteps=10000, measurement_freq=1, temporal_avg=True):
    config = {}
    config["circuit_type"] = "nonlocal_neural"
    config["num_runs"] = num_runs

    config["system_size"] = system_size
    config["eta"] = eta
    config["alpha"] = alpha
    config["J"] = J
    config["fully_connected"] = fully_connected

    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = measurement_freq
    config["temporal_avg"] = temporal_avg

    return config


if __name__ == "__main__":
    system_sizes = [16, 32, 64, 128, 256, 512]
    eta = list(np.linspace(0.0, 0.5, 50))
    alpha = [3.5, 4.0, 4.5, 5.0]
    param_bundle = generate_config(system_sizes, eta, alpha, fully_connected=1, num_runs=50, equilibration_timesteps=0, sampling_timesteps=10000, measurement_freq=10, temporal_avg=False)
    #submit_jobs(f"nonlocal_neural_fc2", param_bundle=param_bundle, ncores=64, memory="40gb", time="24:00:00", nodes=6, cleanup=False)

    system_sizes = [16, 32, 64, 128, 256, 512]
    eta = list(np.linspace(0.0, 1.0, 50))
    param_bundle = generate_config(system_sizes, eta, alpha=0.0, fully_connected=-1, num_runs=50, equilibration_timesteps=0, sampling_timesteps=10000, measurement_freq=10, temporal_avg=False)
    submit_jobs(f"nonlocal_neural_l", param_bundle=param_bundle, ncores=64, memory="40gb", time="24:00:00", nodes=6, cleanup=False)

    system_sizes = [16, 32, 64, 128, 256, 512]
    eta = list(np.linspace(0.0, 1.0, 50))
    param_bundle = generate_config(system_sizes, eta, alpha=0.0, fully_connected=2, num_runs=50, equilibration_timesteps=0, sampling_timesteps=10000, measurement_freq=10, temporal_avg=False)
    submit_jobs(f"nonlocal_neural_cw", param_bundle=param_bundle, ncores=64, memory="40gb", time="24:00:00", nodes=6, cleanup=False)
