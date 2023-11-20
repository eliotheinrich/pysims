from job_controller import config_to_string, submit_jobs, save_config
import numpy as np

def phaseless_config(system_size, dim=1):
    if not isinstance(system_size, list):
        system_size = [system_size]
    
    config = {}
    config["circuit_type"] = "phaseless"
    config["num_runs"] = 1
    
    config["system_size"] = system_size

    config["dim"] = dim
    
    config["spatial_avg"] = True
    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = True
    
    config["mzr_prob"] = np.linspace(0.0, 1.0, 5)
    config["z_prob"] = np.linspace(0.0, 1.0, 5)
    config["num_x_eigenstates"] = list(range(0, max(system_size), max(system_size)//8))
    config["sample_measurement_outcomes"] = True

    #config["sample_fixed_mutual_information"] = True
    #mutual_information_zparams = []
    #for L in system_sizes:
    #    params = {
    #        "system_size": L, 
    #        "x1": 0, 
    #        "x2": L//8, 
    #        "x3": L - L//8,
    #        "x4": L,
    #    }
    #        
    #    mutual_information_zparams.append(params)

    #config["zparams1"] = mutual_information_zparams

    config["temporal_avg"] = False
    config["sampling_timesteps"] = 500
    config["equilibration_timesteps"] = 0
    config["measurement_freq"] = 20

    config["spacing"] = 5

    return config_to_string(config)

if __name__ == "__main__":
    config = phaseless_config(16, dim=1)
    submit_jobs(config, f"phaseless_1_144", ncores=1, memory="20gb", time="24:00:00", nodes=1, run_local=True)

    #config = phaseless_config(144, dim=2)
    #submit_jobs(config, f"phaseless_2_144", ncores=64, memory="100gb", time="24:00:00", nodes=1)