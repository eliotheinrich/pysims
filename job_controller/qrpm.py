from job_controller import submit_jobs, save_config
import numpy as np

def generate_config_very_high_fidelity(
        feedback_mode, 
        us, 
        unitary_qubits=4, 
        nruns=25, 
        system_sizes=[128], 
        initial_state=0,
        simulator_type="chp",
        scrambling_steps=None,
        sample_avalanches=False, 
        sample_structure=True, 
        sample_variable_mutual_information=False,
        equilibration_timesteps=1000,
        sampling_timesteps=2000
    ):

    if scrambling_steps is None:
        scrambling_steps = max(system_sizes)

    config = {}
    config["circuit_type"] = "sandpile_clifford"
    config["num_runs"] = nruns
    config["simulator_type"] = simulator_type
    
    config["random_sites"] = True
    config["boundary_conditions"] = "obc1"
    config["unitary_qubits"] = unitary_qubits
    config["mzr_mode"] = 1

    config["initial_state"] = initial_state
    config["scrambling_steps"] = scrambling_steps

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
    
    config["pbc"] = True
    config["sample_variable_mutual_information"] = sample_variable_mutual_information

    config["sample_surface"] = True
    config["sample_surface_avg"] = True
    config["sample_rugosity"] = True
    config["sample_roughness"] = True
    config["sample_avalanche_sizes"] = sample_avalanches
    config["sample_structure_function"] = sample_structure

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
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = 5

    config["spacing"] = 5
    config["feedback_mode"] = feedback_mode

    return config

def generate_config_very_high_fidelity_temporal(
        feedback_mode, 
        us,
        num_runs=250, 
        simulator_type="chp",
        system_sizes=[128],
        sampling_timesteps=50, 
        sampling_freq=1, 
        initial_state=0,
        scrambling_steps=None,
        sample_staircases=False
    ):

    if scrambling_steps is None:
        scrambling_steps = max(system_sizes)
        
    config = {}
    config["circuit_type"] = "sandpile_clifford"
    config["num_runs"] = num_runs
    config["simulator_type"] = simulator_type

    config["system_size"] = system_sizes
    
    config["random_sites"] = True
    config["boundary_conditions"] = "obc1"
    config["unitary_qubits"] = 4
    config["mzr_mode"] = 1
    
    config["initial_state"] = initial_state
    config["scrambling_steps"] = scrambling_steps

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
    config["sample_staircases"] = sample_staircases

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
    config["measurement_freq"] = sampling_freq

    config["spacing"] = 5
    config["feedback_mode"] = feedback_mode

    return config 

if __name__ == "__main__":
    # revisit 3, 23
    modes = { 1: (5, 15), 2: (2.5, 4.0), 3: (2.5, 3.5), 4: (2.5, 3.5), 5: (2, 5), 6: (2.5, 4), 7: (1, 2.5), 8: (2.0, 3.0), 9: (1.5, 3.75), 10: (1.1, 1.3), 11: (1.3, 2.0), 12: (1.0, 3), 13: (0.1, 1.0), 14: (0.7, 1.6), 15: (0.1, 1.0), 16: (0.75, 1.2), 17: (1.0, 1.75), 18: (1.0, 3.0), 19: (0.05, 0.75), 20: (0.4, 1.5), 21: (0.01, 0.5), 22: (0.2, 0.4), 23: (0.4, 1.5), 24: (0.01, 0.5), 25: (0.001, 0.6), 26: (0.01, 0.25), 27: (0.3, 1.0), 28: (0.01, 0.3), 29: (0.001, 0.2), 30: (0.001, 0.2)}

    #modes_critical = {
        #10: (1.1, 1.3, 500),
    #    20: (0.55, 1.1, 5000),
        #22: (0.2, 0.4, 500),
        #6: (3.2, 3.6),
        #7: (1.6, 2.0),
        #11: (1.6, 1.9),
        #13: (0.45, 0.75),
        #16: (0.85, 1.25),
        #17: (1.25, 1.6),
        #19: (0.25, 0.5),
        #26: (0.08, 0.3)
    #}

    modes = {
        #3: (1.0, 30.0, 3000),
        #4: (1.0, 30.0, 3000),
        #19: (0.1, 10.0, 3000), 
        #30: (0.1, 0.5, 3000),
        #10: (0.6, 1.8, 3000), # Some more 2nd order modes needed

        #14: (0.6, 1.3, 3000),
        #15: (0.4, 1.6, 3000), # Maybe redo, split across more nodes

        #20: (0.3, 1.6, 3000),
        20: (0.5, 0.7, 3000),
        #21: (0.2, 1.2, 3000),
        
        #23: (0.5, 1.0, 3000),
        #24: (0.1, 1.0, 3000),
        
        #27: (0.4, 0.9, 3000),
        #28: (0.1, 1.0, 3000),
    }

    # For rep_mode Fig.
    #modes = {
    #    10: (0.6, 1.8, 3000),
    #    20: (0.4, 1.5, 3000),
    #    30: (0.01, 1.0, 3000)
    #}
   

    us = list(np.linspace(0.05, 2.0, 120))
    eq_timesteps = 3000
        
    system_sizes = [32, 64, 128]
    config = generate_config_very_high_fidelity(list(range(1, 31)), us, system_sizes=system_sizes, nruns=1, sample_avalanches=False, equilibration_timesteps=eq_timesteps, initial_state=0)
    #submit_jobs(config, f"qrpm_all_s3", ncores=64, memory="250gb", time="96:00:00", nodes=5)

    # General data for critical exponent determination
    # Mostly near critical point. 
    # Record error for KPZ fluctuation calculations
    for mode, (umin, umax, eq_timesteps) in modes.items():
        res = 80 if mode not in [] else 40
        res = int(res // 2)
        us = list(np.linspace(umin, umax, res))
        
        system_sizes = [32, 64, 128, 256]
        config = generate_config_very_high_fidelity(mode, us, system_sizes=system_sizes, nruns=5, sample_avalanches=False, equilibration_timesteps=eq_timesteps, initial_state=1, scrambling_steps=1000)
        submit_jobs(config, f"qrpm_{mode}_s_c", ncores=64, memory="250gb", time="96:00:00", nodes=5)

        system_sizes = [512]
        config = generate_config_very_high_fidelity(mode, us, system_sizes=system_sizes, nruns=1, sample_avalanches=False, equilibration_timesteps=eq_timesteps, initial_state=1, scrambling_steps=1000)
        #submit_jobs(config, f"qrpm_{mode}_s2_c", ncores=48, memory="250gb", time="96:00:00", nodes=25)

        system_sizes = [32, 64, 128, 256]
        config = generate_config_very_high_fidelity_temporal(mode, us, num_runs=500, system_sizes=system_sizes, sampling_timesteps=100)
        #submit_jobs(config, f"qrpm_{mode}_t_c", ncores=64, memory="250gb", time="96:00:00", nodes=8)

        system_sizes = [512]
        config = generate_config_very_high_fidelity_temporal(mode, us, num_runs=500, system_sizes=system_sizes, sampling_timesteps=100)
        #submit_jobs(config, f"qrpm_{mode}_t2_c", ncores=64, memory="250gb", time="96:00:00", nodes=8)
    
    modes = {
        #15: (0.001, 0.02, 1000),
        #19: (0.001, 1.0, 1000),
        21: (0.001, 1.0, 1000),
        24: (0.001, 1.0, 1000),
        #25: (0.001, 1.0, 1000),
        28: (0.001, 1.0, 1000),
        #29: (0.001, 1.0, 1000),
        #30: (0.001, 1.0, 1000)
    }
    # Check if trivial modes can reverse scrambling
    for mode, (umin, umax, eq_timesteps) in modes.items():
        res = 40
        us = list(np.linspace(umin, umax, res))
        
        system_sizes = [128]
        config = generate_config_very_high_fidelity(
            mode, 
            us, 
            system_sizes=system_sizes, 
            nruns=10, 
            sample_avalanches=False, 
            equilibration_timesteps=eq_timesteps, 
            initial_state=1, 
            scrambling_steps=512
        )
        
        # DESCRAMBLING
        #submit_jobs(config, f"qrpm_{mode}_s_sc", ncores=64, memory="250gb", time="96:00:00", nodes=1)

    modes_broad = {
        10: (0.5, 2.5, 2000),
        #20: (0.2, 2.0, 2000),
        #22: (0.1, 1.5, 2000),
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
        #submit_jobs(config, f"qrpm_{mode}_s2", ncores=64, memory="150gb", time="72:00:00", nodes=4)
        
        system_sizes = [32, 64, 128]
        config = generate_config_very_high_fidelity(mode, us, simulator_type="chp", system_sizes=system_sizes, sample_avalanches=True, sample_variable_mutual_information=True, equilibration_timesteps=eq_timesteps)
        #submit_jobs(config, f"qrpm_{mode}_av_test", ncores=64, memory="150gb", time="24:00:00", nodes=1, cleanup=False)

    system_sizes = []


    modes_very_broad = {
        10: (0.1, 3.0, 3000),
        20: (0.1, 3.0, 3000), # Avalanches might be interesting in intermediate regime but don't care about might higher
        21: (0.1, 3.0, 3000),
        #6: (3.2, 3.6),
        #7: (1.6, 2.0),
        #11: (1.6, 1.9),
        #13: (0.45, 0.75),
        #16: (0.85, 1.25),
        #17: (1.25, 1.6),
        #19: (0.25, 0.5),
        #26: (0.08, 0.3)
    }

    for mode, (umin, umax, eq_timesteps) in modes_very_broad.items():
        us = list(np.linspace(umin, umax, 80))
        #us.append(100)
        #us.append(1000)

        system_sizes = [128]
        config = generate_config_very_high_fidelity(mode, us, system_sizes=system_sizes, nruns=25, sample_avalanches=True, simulator_type="chp", equilibration_timesteps=eq_timesteps)
        #submit_jobs(config, f"qrpm_{mode}_av", ncores=64, memory="150gb", time="72:00:00", nodes=8)
        
        system_sizes = [32, 64, 128, 256]
        config = generate_config_very_high_fidelity(mode, us, system_sizes=system_sizes, sample_avalanches=False, equilibration_timesteps=eq_timesteps)
        #submit_jobs(config, f"qrpm_{mode}_s3", ncores=64, memory="150gb", time="72:00:00", nodes=4)

        system_sizes = [32, 64, 256]
        config = generate_config_very_high_fidelity_temporal(mode, us, num_runs=1250, system_sizes=system_sizes, sampling_timesteps=100)
        #submit_jobs(config, f"qrpm_{mode}_tkpz", ncores=48, memory="150gb", time="72:00:00", nodes=4)
        
    us = np.linspace(0.001, 0.2, 40)
    config = generate_config_very_high_fidelity(30, us, system_sizes=[32, 64, 128], equilibration_timesteps=[50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000], sampling_timesteps=[50], nruns=10)
    #submit_jobs(config, f"qrpm_30_s_eq", ncores=64, memory="250gb", time="48:00:00", nodes=8)
