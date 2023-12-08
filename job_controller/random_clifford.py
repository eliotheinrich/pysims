from job_controller import config_to_string, submit_jobs, save_config
import numpy as np

def generate_config_very_high_fidelity(system_sizes=[128], simulator_type="chp", mzr_probs=None, sample_structure_function=True, sample_variable_mutual_information=False):
    config = {}
    config["circuit_type"] = "random_clifford"
    config["num_runs"] = 500
    config["simulator_type"] = simulator_type

    config["gate_width"] = 2

    config["pbc"] = [True]
    
    # EntropySampler settings
    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = False
    config["sample_mutual_information"] = False
    config["sample_fixed_mutual_information"] = True
    config["sample_variable_mutual_information"] = sample_variable_mutual_information
    
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

    # InterfaceSampler settings
    config["sample_surface"] = True
    config["sample_surface_avg"] = True
    config["sample_rugosity"] = True
    config["sample_roughness"] = True
    config["sample_avalanche_sizes"] = False
    config["sample_structure_function"] = sample_structure_function

    if mzr_probs is None:
        config["mzr_prob"] = [0.00, 0.02, 0.04, 0.06, 0.08, 0.10, 0.12, 0.14, 0.15, 0.16, 0.17, 0.18, 0.20, 0.22, 0.24, 0.26, 0.28, 0.30, 0.50, 1.00]
    else:
        config["mzr_prob"] = mzr_probs

    config["temporal_avg"] = True
    config["sampling_timesteps"] = 100
    config["equilibration_timesteps"] = 50
    config["measurement_freq"] = 10

    config["spacing"] = 5

    return config_to_string(config)

def generate_config_very_high_fidelity_temporal(system_sizes=[128], simulator_type="chp", mzr_probs=None):
    if mzr_probs is None:
        mzr_probs = [0.00, 0.02, 0.04, 0.06, 0.08, 0.10, 0.12, 0.14, 0.15, 0.16, 0.17, 0.18, 0.20, 0.22, 0.24, 0.26, 0.28, 0.30, 0.50, 1.00]
    config = {}
    config["circuit_type"] = "random_clifford"
    config["num_runs"] = 1
    config["simulator_type"] = simulator_type

    config["gate_width"] = 2

    config["pbc"] = [True]
    config["seed"] = 315
    
    config["system_size"] = system_sizes
    
    config["sample_entropy"] = False

    config["mzr_prob"] = mzr_probs

    config["spatial_avg"] = False
    config["temporal_avg"] = False
    config["equilibration_timesteps"] = 0
    config["measurement_freq"] = 1
    config["sampling_timesteps"] = 100

    config["spacing"] = 5

    return config_to_string(config)

if __name__ == "__main__":
    #config = generate_config_very_high_fidelity_temporal(system_sizes=[128, 256])
    #submit_jobs(config, f"rc_t_256", ncores=64, memory="150gb", time="48:00:00", nodes=4)

    system_sizes = [32]
    L = max(system_sizes)
    config = generate_config_very_high_fidelity(system_sizes=system_sizes, sample_structure_function=False, sample_variable_mutual_information=[True,False])
    save_config(config, "../rc_config.json")
    #submit_jobs(config, f"rc_vmi_small", ncores=12, nodes=1, memory="10gb", time="48:00:00", record_error=True, cleanup=True)

    
   #config = generate_config_very_high_fidelity_temporal(system_sizes=[128], simulator_type=["chp"])
   #submit_jobs(config, 'rc_test', ncores=2, run_local=True)
   #submit_jobs(config, f"rc_test", ncores=4, nodes=1, memory="10gb", time="48:00:00", record_error=False, run_local=True)
    
    config = generate_config_very_high_fidelity(system_sizes=[16], simulator_type=["graph"], sample_structure_function=False)
    #save_config(config, "../configs/rc_test.json")
    #submit_jobs(config, f"rc_test_", ncores=4, nodes=1, memory="10gb", time="48:00:00", record_error=False, run_local=True)