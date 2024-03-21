from job_controller import config_to_string, submit_jobs
import numpy as np

def generate_test_config(system_sizes=[128], circuit_depths=None, mzr_probs=None, nruns=1):
    config = {}
    config["circuit_type"] = "bulk_sim"
    config["num_runs"] = nruns
    
    if mzr_probs is None:
        mzr_probs = [1.0]    
    config["mzr_prob"] = mzr_probs
    if circuit_depths is None:
        circuit_depths = system_sizes


    config["circuit_depth"] = circuit_depths

    config["system_size"] = system_sizes

    config["temporal_avg"] = True
    config["sampling_timesteps"] = 100
    config["equilibration_timesteps"] = 0
    config["measurement_freq"] = 1

    return config

if __name__ == "__main__":
    system_sizes = 1024
    mzr_probs = list(np.linspace(0.1, 1.0, 20))
    circuit_depths = list(range(0, 4096, 64))
    nruns = 5
    config = generate_test_config(system_sizes=[system_sizes], circuit_depths=circuit_depths, mzr_probs=mzr_probs, nruns=nruns)
    submit_jobs(config, f"bulk_test_1024", ncores=64, memory="150gb", time="10:00:00", nodes=10)