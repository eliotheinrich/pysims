from job_controller import config_to_string, submit_jobs
import numpy as np

def generate_config_very_high_fidelity(feedback_mode, us, unitary_qubits=4, system_sizes=[128], sample_avalanches=False, equilibration_timesteps=1000):
    config = {}
    config["circuit_type"] = "sandpile_clifford"
    config["num_runs"] = 25
    config["serialize"] = True
    
    config["random_sites"] = True
    config["boundary_conditions"] = "obc1"
    config["unitary_qubits"] = unitary_qubits
    config["mzr_mode"] = 1

    config["spatial_avg"] = False
    
    # EntropySampler configuration
    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = False
    config["sample_mutual_information"] = False
    config["sample_fixed_mutual_information"] = True
    mutual_information_zparams = []
    for L in system_sizes:
        params = {
            # Mutual information
            "system_size": L, 
            "x1": 0, 
            "x2": L//8, 
            "x3": L - L//8,
            "x4": L,
            # Avalanches
            "num_bins": 2*L,
            "max_av": 2*L,
            "min_av": 1
        }
            
        mutual_information_zparams.append(params)
    
    config["zparams1"] = mutual_information_zparams

    config["sample_surface"] = True
    config["sample_surface_avg"] = True
    config["sample_rugosity"] = True
    config["sample_roughness"] = True
    config["sample_avalanche_size"] = sample_avalanches
    config["sample_structure_function"] = True

    probs = []
    for u in us:
        probs_u = {}
        if u < 1.0:
            probs_u['unitary_prob'] = u
            probs_u['mzr_prob'] = 1.0
        else:
            probs_u['unitary_prob'] = 1.0
            probs_u['mzr_prob'] = 1.0/u
        probs.append(probs_u)
    
    config["zparams2"] = probs

    config["temporal_avg"] = True
    config["sampling_timesteps"] = 2000
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = 5

    config["spacing"] = 5
    config["feedback_mode"] = feedback_mode

    return config_to_string(config)

def generate_config_very_high_fidelity_temporal(feedback_mode, us, num_runs=250, system_sizes=[128], sampling_timesteps=50):
    config = {}
    config["circuit_type"] = "sandpile_clifford"
    config["num_runs"] = num_runs

    config["system_size"] = system_sizes
    
    config["random_sites"] = True
    config["boundary_conditions"] = "obc1"
    config["unitary_qubits"] = 4
    config["mzr_mode"] = 1

    config["spatial_avg"] = False

    # Disable EntropySampler
    config["sample_entropy"] = False
    config["sample_all_partition_sizes"] = False
    config["sample_mutual_information"] = False
    config["sample_fixed_mutual_information"] = False
    
    # Sample surface using InterfaceSampler
    config["sample_surface"] = True
    config["sample_surface_avg"] = True
    config["sample_rugosity"] = False
    config["sample_roughness"] = True
    config["sample_avalanche_size"] = False
    config["sample_structure_function"] = False

    probs = []
    for u in us:
        probs_u = {}
        if u < 1.0:
            probs_u['unitary_prob'] = u
            probs_u['mzr_prob'] = 1.0
        else:
            probs_u['unitary_prob'] = 1.0
            probs_u['mzr_prob'] = 1.0/u
        probs.append(probs_u)
    
    config["zparams2"] = probs

    config["temporal_avg"] = False
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = 0
    config["measurement_freq"] = 1

    config["spacing"] = 5
    config["feedback_mode"] = feedback_mode

    return config_to_string(config)

if __name__ == "__main__":
    modes = { 1: (1, 25), 2: (1, 5), 3: (1, 4), 4: (2, 4), 5: (2, 5), 6: (3, 6), 7: (1, 5), 8: (1.5, 3.5), 9: (2, 4), 10: (0.5, 4.0), 11: (1.0, 3.0), 12: (1.5, 3.5), 13: (0.5, 3.0), 14: (0.1, 2.0), 15: (0.5, 1.5), 16: (0.5, 1.5), 17: (0.5, 2.5), 18: (1.0, 3.0), 19: (0.1, 2.5), 20: (0.4, 1.5), 21: (0.1, 1.0), 22: (0.1, 1.0), 23: (0.4, 1.5), 24: (0.1, 1.0), 25: (0.001, 0.6), 26: (0.1, 0.5), 27: (0.3, 1.0), 28: (0.2, 0.6), 29: (0.001, 0.4), 30: (0.001, 0.4)}

# waiting for 
#	high res timeseries: t
#	high res spaceseries (with surface info): s1
#	low res spaceseries (with surface info): s2
#	low res avalanche distribution: av

    modes_critical = {
        10: (1.1, 1.3, 500),
        20: (0.4, 1.3, 2000),
        22: (0.2, 0.4, 500),
        #6: (3.2, 3.6),
        #7: (1.6, 2.0),
        #11: (1.6, 1.9),
        #13: (0.45, 0.75),
        #16: (0.85, 1.25),
        #17: (1.25, 1.6),
        #19: (0.25, 0.5),
        #26: (0.08, 0.3)
    }


    # General data for critical exponent determination
    # Mostly near critical point. 
    # Record error for KPZ fluctuation calculations
    for mode, (umin, umax, eq_timesteps) in modes_critical.items():
        us = list(np.linspace(umin, umax, 80))
        
        system_sizes = [32, 64, 128, 256]
        config = generate_config_very_high_fidelity(mode, us, system_sizes=system_sizes, sample_avalanches=False, equilibration_timesteps=eq_timesteps)
        #submit_jobs(config, f"qrpm_{mode}_s1", ncores=64, memory="150gb", time="72:00:00", nodes=4, record_error=True)

        system_sizes = [32, 64, 128, 256]
        config = generate_config_very_high_fidelity_temporal(mode, us, num_runs=1250, system_sizes=system_sizes, sampling_timesteps=100)
        #submit_jobs(config, f"qrpm_{mode}_t", ncores=64, memory="250gb", time="72:00:00", nodes=8, record_error=True)






    modes_broad = {
        10: (0.1, 1.5, 2000),
        20: (0.1, 2.0, 2000),
        22: (0.1, 0.6, 2000),
        #6: (3.2, 3.6),
        #7: (1.6, 2.0),
        #11: (1.6, 1.9),
        #13: (0.45, 0.75),
        #16: (0.85, 1.25),
        #17: (1.25, 1.6),
        #19: (0.25, 0.5),
        #26: (0.08, 0.3)
    }
    
    # Avalanche distribution. 
    # Smaller system size, far away from criticality
    for mode, (umin, umax, eq_timesteps) in modes_broad.items():
        us = list(np.linspace(umin, umax, 40))
        
        system_sizes = [32, 64, 128, 256]
        config = generate_config_very_high_fidelity(mode, us, system_sizes=system_sizes, sample_avalanches=False, equilibration_timesteps=eq_timesteps)
        #submit_jobs(config, f"qrpm_{mode}_s2", ncores=64, memory="150gb", time="72:00:00", nodes=4, record_error=True)

        system_sizes = [32, 64, 128]
        config = generate_config_very_high_fidelity(mode, us, system_sizes=system_sizes, sample_avalanches=True, equilibration_timesteps=eq_timesteps)
        submit_jobs(config, f"qrpm_{mode}_av", ncores=64, memory="150gb", time="72:00:00", nodes=4, record_error=True)