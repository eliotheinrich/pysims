import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from dataframe import *
from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, beta, xx_prob, measurement_type=1, unitary_type=1, sre_type="montecarlo", num_runs=1):
    # Default to weak measurements and clifford unitaries
    config = {}
    config["circuit_type"] = "mps_simulator"
    config["num_runs"] = num_runs

    system_size = list(system_size)
    zparams = [{"system_size": L, "magic_mutual_information_subsystem_size": L//8} for L in system_size]
    config["bond_dimension"] = 50
    config["beta"] = beta
    config["xx_prob"] = xx_prob

    config["unitary_type"] = unitary_type
    config["measurement_type"] = measurement_type

    #config["sample_magic_mutual_information"] = False
    #config["sample_stabilizer_renyi_entropy"] = True
    config["sre_mc_equilibration_timesteps"] = 500
    config["sre_method"] = sre_type
    #config["sre_num_samples"] = 5000
    config["zparams2"] = zparams

    config["sample_surface"] = True

    config["temporal_avg"] = False
    config["equilibration_timesteps"] = 0
    config["sampling_timesteps"] = 30
    config["measurement_freq"] = 1

    return config

if __name__ == "__main__":
    L = [8, 16]#, 32, 64]
    beta = [0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0]
    xx_prob = np.linspace(0.0, 1.0, 50)
    for m in [1]:
        for u in [1]:
            param_matrix = generate_config(L, beta=beta, xx_prob=xx_prob, measurement_type=m, unitary_type=u, sre_type="virtual", num_runs=1)
            submit_jobs(f"xxz_test_entanglement", param_bundle=param_matrix, ncores=64, memory="10gb", time="24:00:00", nodes=1, cleanup=False, run_local=False)

