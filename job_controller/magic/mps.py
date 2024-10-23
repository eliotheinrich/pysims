import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from dataframe import *
from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, mzr_prob, xx_prob, sre_type="montecarlo", num_runs=1):
    config = {}
    config["circuit_type"] = "mps_simulator"
    config["num_runs"] = num_runs

    system_size = list(system_size)
    zparams = [{"system_size": L, "magic_mutual_information_subsystem_size": L//8} for L in system_size]
    config["bond_dimension"] = 50
    config["mzr_prob"] = mzr_prob
    config["xx_prob"] = xx_prob

    config["sample_magic_mutual_information"] = True
    config["sample_stabilizer_renyi_entropy"] = True
    config["sre_mc_equilibration_timesteps"] = 5000
    config["sre_method"] = sre_type
    config["sre_num_samples"] = 5000
    config["zparams"] = zparams

    config["temporal_avg"] = False
    config["equilibration_timesteps"] = 0
    config["sampling_timesteps"] = 1000
    config["measurement_freq"] = 1

    return config


if __name__ == "__main__":
    L = [16, 32, 64]
    mzr_prob = 1.0
    xx_prob = np.linspace(0.0, 1.0, 50)
    param_matrix = generate_config(L, mzr_prob=mzr_prob, xx_prob=xx_prob, sre_type="montecarlo", num_runs=1)
    submit_jobs(f"xxz_test", param_bundle=param_matrix, ncores=1, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=True)

