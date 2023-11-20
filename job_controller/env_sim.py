from job_controller import config_to_string, submit_jobs
import numpy as np

def generate_test_config(ps, dim=1, system_sizes=[128]):
    config = {}
    config["circuit_type"] = "env_sim"
    config["num_runs"] = 100
    
    config["env_dim"] = dim
    config["int_prob"] = ps

    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = True
    config["sample_fixed_mutual_information"] = True
    
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
    config["sampling_timesteps"] = 2000
    config["equilibration_timesteps"] = 1000
    config["measurement_freq"] = 5

    config["spacing"] = 5

    return config_to_string(config)

if __name__ == "__main__":
    system_sizes = [16, 32, 64]
    ps = np.linspace(0.001, .2, 25)
    config = generate_test_config(ps, dim=1, system_sizes=system_sizes)
    submit_jobs(config, f"env_test", ncores=64, memory="150gb", time="72:00:00", nodes=4)
