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
        sample_structure=False,
        sample_variable_mutual_information=False,
        equilibration_timesteps=1000,
        sampling_timesteps=2000,
        temporal_avg=True,
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
    config["temporal_avg"] = temporal_avg
    config["sampling_timesteps"] = sampling_timesteps
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = 5

    config["spacing"] = 5
    config["feedback_mode"] = feedback_mode

    return config

def modify_config1(c):
    c["equilibration_timesteps"] = 100
    c["sampling_timesteps"] = 400
    c["measurement_freq"] = 1

    #print(f"After first modification: {c}")

def modify_config2(c):
    c["equilibration_timesteps"] = 0
    c["sampling_timesteps"] = 1000
    c["measurement_freq"] = 1

    #print(f"After second modification: {c}")

if __name__ == "__main__":
    # ALL MODES
    us = list(np.linspace(0.05, 2.0, 5))
    eq_timesteps = 200

    system_sizes = [8]
    callbacks = [modify_config1, modify_config2]
    config = generate_config([10], us, system_sizes=system_sizes, nruns=1, sample_avalanches=False, equilibration_timesteps=eq_timesteps, sampling_timesteps=0, temporal_avg=True, initial_state=0)

    nodes = 2
    submit_jobs("_qrpm_test", param_bundle=config, ncores=4, memory="1gb", time="00:10:00", nodes=nodes, run_local=False, cleanup=False, num_checkpoints=2, checkpoint_callbacks=callbacks)
