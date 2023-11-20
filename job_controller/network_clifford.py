from job_controller import config_to_string, submit_jobs, save_config
import numpy as np

def nc_config(system_size):
    config = {"circuit_type": "network_clifford"}
    config["num_runs"] = 1000

    config["system_size"] = system_size

    config["mzr_prob"] = 0.0
    config["p"] = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
    config["alpha"] = [0.25, 0.5, 0.75, 1.25, 1.5, 3.0]

    config["temporal_avg"] = False
    config["sampling_timesteps"] = 50
    config["equilibration_timesteps"] = 0
    config["measurement_freq"] = 1
    
    return config_to_string(config)

if __name__ == "__main__":
    config = nc_config(128)
    submit_jobs(config, f"test_nc", ncores=64, memory="20gb", time="24:00:00")
