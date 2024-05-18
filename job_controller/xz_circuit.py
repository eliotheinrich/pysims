from job_controller import config_to_string, submit_jobs, save_config
import numpy as np

def xz_config(
        system_sizes, 
        mzr_probs, 
        pz,
        dim=0,
        initial_state=0,
        random_sites=False,
        sampling_timesteps=0, 
        equilibration_timesteps=0, 
        measurement_freq=5,
        interface_sample=True,
        nruns=500,
    ):

    config = {}
    config["circuit_type"] = "xz_circuit"
    config["num_runs"] = nruns

    config["mzr_prob"] = mzr_probs
    config["pz"] = pz
    config["dim"] = dim
    config["initial_state"] = initial_state
    config["random_sites"] = random_sites

    # EntropySampler settings
    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = False
    config["sample_mutual_information"] = False
    config["sample_fixed_mutual_information"] = False
    config["sample_variable_mutual_information"] = False 
    config["sample_correlation_distance"] = False 
    
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
    config["sample_surface"] = interface_sample 
    config["sample_surface_avg"] = interface_sample 
    config["sample_rugosity"] = interface_sample
    config["sample_roughness"] = interface_sample
    config["sample_avalanche_sizes"] = False
    config["sample_structure_function"] = False

    config["temporal_avg"] = False
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = measurement_freq

    return config


system_sizes = [128]
mzr_probs = list(np.linspace(0.05, 1.0, 10)) 
pz = list(np.linspace(0.05, 1.0, 10))
config = xz_config(system_sizes=system_sizes, mzr_probs=mzr_probs, pz=pz, dim=[0,1], initial_state=[0,1,2], random_sites=[0,1], sampling_timesteps=500, equilibration_timesteps=0, measurement_freq=5, nruns=10, interface_sample=True)
submit_jobs(config, f"xz_circuit", ncores=64, nodes=1, memory="20gb", time="05:00:00", run_local=False)

