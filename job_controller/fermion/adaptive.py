import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs, save_config
import numpy as np

def generate_config(
        system_sizes=[128], 
        p=0.5, 
        r=0.5,
        nruns=500,
        sampling_timesteps=None, 
        sample_fixed_mutual_information=False,
        sample_variable_mutual_information=False,
        equilibration_timesteps=None, 
        sample_correlation_distance=False
    ):

    config = {}
    config["num_runs"] = nruns

    config["circuit_type"] = "adaptive_fermion"
    config["p"] = p
    config["r"] = r

    config["sample_correlations"] = True

    # EntropySampler settings
    config["sample_entropy"] = True 
    config["renyi_indices"] = "1,2"
    config["sample_all_partition_sizes"] = True 
    config["sample_mutual_information"] = False
    config["sample_fixed_mutual_information"] = sample_fixed_mutual_information
    config["sample_variable_mutual_information"] = sample_variable_mutual_information
    config["spacing"] = 8

    mutual_information_zparams = []
    for L in system_sizes:
        params = {
            "system_size": L, 
            "x1": 0, 
            "x2": L//8, 
            "x3": L//2,
            "x4": L//2 + L//8,
        }

        mutual_information_zparams.append(params)

    config["zparams1"] = mutual_information_zparams

    config["temporal_avg"] = True
    config["sampling_timesteps"] = 3*max(system_sizes) if sampling_timesteps is None else sampling_timesteps
    config["equilibration_timesteps"] = 3*max(system_sizes) if equilibration_timesteps is None else equilibration_timesteps
    config["measurement_freq"] = 5

    config["spacing"] = 5

    return config


if __name__ == "__main__":
    system_sizes = [64]
    p = 0.5
    r = 0.5
    config = generate_config(system_sizes=system_sizes, p=p, r=r, nruns=4)
    print(config)
    submit_jobs(f"adaptive_fermion", param_bundle=config, ncores=4, nodes=1, memory="50gb", time="48:00:00", run_local=True, cleanup=False)


