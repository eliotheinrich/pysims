from job_controller import submit_jobs, save_config, unbundle_params
import numpy as np

def generate_config_very_high_fidelity(
        system_sizes=[128], 
        simulator_type="chp", 
        mzr_probs=None, 
        nruns=500,
        timestep_type=0, 
        alpha=2.0, 
        sampling_timesteps=None, 
        equilibration_timesteps=None, 
        interface_sample=True,
        sample_structure_function=True, 
        sample_fixed_mutual_information=True,
        sample_variable_mutual_information=False,
        sample_correlation_distance=False
    ):

    config = {}
    config["circuit_type"] = "random_clifford"
    config["num_runs"] = nruns
    config["simulator_type"] = simulator_type

    config["gate_width"] = 2
    config["timestep_type"] = timestep_type
    config["alpha"] = alpha

    config["pbc"] = True
    
    # EntropySampler settings
    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = False
    config["sample_mutual_information"] = False
    config["sample_fixed_mutual_information"] = sample_fixed_mutual_information
    config["sample_variable_mutual_information"] = sample_variable_mutual_information
    config["sample_correlation_distance"] = sample_correlation_distance
    
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
    config["sample_structure_function"] = sample_structure_function

    if mzr_probs is None:
        config["mzr_prob"] = [0.00, 0.02, 0.04, 0.06, 0.08, 0.10, 0.12, 0.14, 0.15, 0.16, 0.17, 0.18, 0.20, 0.22, 0.24, 0.26, 0.28, 0.30, 0.50, 1.00]
    else:
        config["mzr_prob"] = mzr_probs

    config["temporal_avg"] = True
    config["sampling_timesteps"] = 3*max(system_sizes) if sampling_timesteps is None else sampling_timesteps
    config["equilibration_timesteps"] = 3*max(system_sizes) if equilibration_timesteps is None else equilibration_timesteps
    config["measurement_freq"] = 5

    config["spacing"] = 5

    return config

def generate_config_very_high_fidelity_temporal(system_sizes=[128], simulator_type="chp", mzr_probs=None, nruns=1, timestep_type=0, alpha=2.0, measurement_freq=1, sampling_timesteps=100):
    if mzr_probs is None:
        mzr_probs = [0.00, 0.02, 0.04, 0.06, 0.08, 0.10, 0.12, 0.14, 0.15, 0.16, 0.17, 0.18, 0.20, 0.22, 0.24, 0.26, 0.28, 0.30, 0.50, 1.00]
    config = {}
    config["circuit_type"] = "random_clifford"
    config["num_runs"] = nruns
    config["simulator_type"] = simulator_type
    config["timestep_type"] = timestep_type
    config["alpha"] = alpha

    config["gate_width"] = 2

    config["pbc"] = True
    
    config["sample_fixed_mutual_information"] = True

    zparams_mi = []
    if isinstance(system_sizes, int):
        system_sizes = [system_sizes]
    for L in system_sizes:
        zparams_mi.append({
            "system_size": L,
            "x1": 0,
            "x2": L//8,
            "x3": L//2,
            "x4": L//2 + L//8
        })
    config["zparams_mi"] = zparams_mi

    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = True

    config["mzr_prob"] = mzr_probs

    config["spatial_avg"] = False
    config["temporal_avg"] = False
    config["equilibration_timesteps"] = 0
    config["measurement_freq"] = measurement_freq
    config["sampling_timesteps"] = sampling_timesteps

    config["spacing"] = 5

    return config

if __name__ == "__main__":
    mzr_probs = np.linspace(0.0, 0.4, 10)
    mzr_probs = [0.16]
    system_sizes = [16, 32, 64, 128]

    param_matrix = generate_config_very_high_fidelity(system_sizes=system_sizes, mzr_probs=mzr_probs, sample_structure_function=False, sample_fixed_mutual_information=False)
    submit_jobs(f"rc_c", param_bundle=param_matrix, ncores=4, nodes=1, memory="10gb", time="48:00:00", run_local=True)


    #def generate_config_very_high_fidelity(
    #        system_sizes=[128], 
    #        simulator_type="chp", 
    #        mzr_probs=None, 
    #        nruns=500,
    #        timestep_type=0, 
    #        alpha=2.0, 
    #        sampling_timesteps=None, 
    #        equilibration_timesteps=None, 
    #        interface_sample=True,
    #        sample_structure_function=True, 
    #        sample_fixed_mutual_information=True,
    #        sample_variable_mutual_information=False,
    #        sample_correlation_distance=False
    #):

    #mzr_probs = [0.0]
    #alphas = 2.0
    #system_sizes = [32]
    #config = generate_config_very_high_fidelity_temporal(system_sizes=system_sizes, mzr_probs=mzr_probs, timestep_type=3, alpha=alphas, nruns=1000, sampling_timesteps=1024)
    #submit_jobs(f"rc_clean_pl", param_bundle=config, ncores=48, nodes=6, memory="50gb", time="48:00:00", run_local=False)

    mzr_probs = [0.0]
    alphas = 2.0
    system_sizes = [16]
    config = generate_config_very_high_fidelity_temporal(system_sizes=system_sizes, mzr_probs=mzr_probs, timestep_type=3, alpha=alphas, nruns=1000, sampling_timesteps=1024)
    #submit_jobs(f"rc_clean", param_bundle=config, ncores=4, nodes=6, memory="50gb", time="48:00:00", run_local=True)

    #mzr_probs = [0.0]
    #config = generate_config_very_high_fidelity_temporal(system_sizes=system_sizes, mzr_probs=mzr_probs, timestep_type=2, nruns=500, sampling_timesteps=2048)
    #submit_jobs(config, f"rc_clean_nl", ncores=48, nodes=1, memory="50gb", time="48:00:00", run_local=False)

    #mzr_probs = [0.0]
    #config = generate_config_very_high_fidelity_temporal(system_sizes=system_sizes, mzr_probs=mzr_probs, timestep_type=1, nruns=500, sampling_timesteps=2048)
    #submit_jobs(config, f"rc_clean_l", ncores=48, nodes=1, memory="50gb", time="48:00:00", run_local=False)

    #system_sizes = [16, 32, 64]
    #mzr_probs = list(np.linspace(0.0, 0.8, 80)) 
    #config = generate_config_very_high_fidelity(system_sizes=system_sizes, mzr_probs=mzr_probs, timestep_type=0, nruns=10, sample_structure_function=False, sample_correlation_distance=True, interface_sample=False, sample_fixed_mutual_information=False)
    #submit_jobs(config, f"rc_cd", ncores=16, nodes=10, memory="20gb", time="05:00:00", run_local=False)

