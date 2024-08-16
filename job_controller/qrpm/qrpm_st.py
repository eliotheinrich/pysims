import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs, save_config
import numpy as np

def generate_config(
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
        sampling_timesteps=2000,
        sample_reduced_surface=True,
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
    config["sample_threshold"] = False 
    config["sample_edges"] = True
    config["sample_reduced_surface"] = sample_reduced_surface

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

    config["save_samples"] = False
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
        measurement_freq=1,
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
    config["sample_roughness"] = False
    config["sample_avalanche_size"] = False
    config["sample_structure_function"] = False
    config["sample_staircases"] = sample_staircases
    config["sample_edges"] = True

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

    config["save_samples"] = False
    config["temporal_avg"] = False 
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = 0
    config["measurement_freq"] = measurement_freq

    config["spacing"] = 1
    config["feedback_mode"] = feedback_mode

    return config 


if __name__ == "__main__":
    # For critical exponent calculations
    modes = {
        10: (1.0, 1.4, 6000),
        20: (0.45, 0.85, 6000),
    }

    for mode, (umin, umax, eq_timesteps) in modes.items():
        res = 20
        if mode == 30:
            res = 10
        us = list(np.linspace(umin, umax, res))

        def start_sampling(param):
            param["equilibration_timesteps"] = 0
            param["sampling_timesteps"] = 3000

        system_sizes = [32, 64, 128, 256]
        config = generate_config(mode, us, system_sizes=system_sizes, nruns=3, sample_avalanches=False, equilibration_timesteps=eq_timesteps, sampling_timesteps=0)
        #submit_jobs(f"qrpm_{mode}_s1_c", param_bundle=config, ncores=64, memory="100gb", time="96:00:00", nodes=20, cleanup=False, checkpoint_callbacks=[start_sampling])

        system_sizes = [512]
        config = generate_config(mode, us, system_sizes=system_sizes, nruns=3, sample_avalanches=False, equilibration_timesteps=eq_timesteps, sampling_timesteps=0)
        #submit_jobs(f"qrpm_{mode}_s2_c", param_bundle=config, ncores=64, memory="100gb", time="96:00:00", nodes=20, cleanup=False, checkpoint_callbacks=[start_sampling])

        system_sizes = [16, 64, 128, 256]
        config = generate_config_very_high_fidelity_temporal(mode, us, num_runs=500, system_sizes=system_sizes, sampling_timesteps=100)
        submit_jobs(f"qrpm_{mode}_t1_c", param_bundle=config, ncores=64, memory="100gb", time="96:00:00", nodes=30, cleanup=False)

        system_sizes = [512]
        config = generate_config_very_high_fidelity_temporal(mode, us, num_runs=500, system_sizes=system_sizes, sampling_timesteps=100)
        submit_jobs(f"qrpm_{mode}_t2_c", param_bundle=config, ncores=64, memory="100gb", time="96:00:00", nodes=30, cleanup=False)
