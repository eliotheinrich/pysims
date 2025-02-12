import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from dataframe import *
from job_controller import submit_jobs
import numpy as np

def generate_param_matrix(system_size, h, state_type=0, delta=1.0, sre_methods="montecarlo", num_runs=1):
    config = {}
    config["config_type"] = "quantum_ising"
    config["num_runs"] = num_runs

    config["system_size"] = system_size
    config["magic_mutual_information_subsystem_size"] = [4]
    config["h"] = h
    config["bond_dimension"] = 50
    config["num_sweeps"] = 500

    config["state_type"] = state_type
    config["delta"] = delta

    config["stabilizer_renyi_indices"] = "2"
    config["sre_mc_equilibration_timesteps"] = 5000
    config["sre_method"] = sre_methods
    config["sre_num_samples"] = 50000

    config["sample_stabilizer_renyi_entropy"] = False 
    config["sample_magic_mutual_information"] = True
    config["sample_bipartite_magic_mutual_information"] = True 

    save_samples = False 
    config["save_sre_samples"] = save_samples
    config["save_mmi_samples"] = save_samples

    return config


if __name__ == "__main__":
    L = [8]
    h = np.linspace(0.0, 2.0, 50)
    for Li in L:
        # NOTE: exhaustive = montecarlo for this funky trial
        sre_methods = ["exhaustive", "exact", "virtual", "montecarlo"]
        param_matrix = generate_param_matrix([Li], h, state_type=0, sre_methods=sre_methods, num_runs=1)
        submit_jobs(f"ising_test{Li}", param_bundle=param_matrix, ncores=60, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=False)

        sre_methods = ["exact"]#, "montecarlo", "virtual"]
        param_matrix = generate_param_matrix([Li], h, state_type=[1], sre_methods=sre_methods, num_runs=1)
        #submit_jobs(f"TEST_ising_test{Li}_mmi", param_bundle=param_matrix, ncores=1, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=True)

