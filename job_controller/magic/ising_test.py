import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, h, state_type=0, delta=1.0, sample_magic=False, sre_type="montecarlo", num_runs=1):
    config = {}
    config["circuit_type"] = "quantum_ising"
    config["num_runs"] = num_runs

    system_size = list(system_size)
    zparams = [{"system_size": L, "magic_mutual_information_subsystem_size": L//8} for L in system_size]
    config["h"] = h
    config["bond_dimension"] = 30

    config["state_type"] = state_type
    config["delta"] = delta

    config["sample_probabilities"] = False
    config["sample_bitstring_distribution"] = False

    config["stabilizer_renyi_indices"] = "2"
    config["sample_stabilizer_renyi_entropy"] = sample_magic
    config["sre_method"] = sre_type
    config["sre_num_samples"] = 50000
    config["sample_magic_mutual_information"] = sample_magic
    config["zparams"] = zparams

    return config


if __name__ == "__main__":
    L = [8, 16, 32]
    h = np.linspace(0.5, 1.5, 50)
    param_matrix = generate_config(L, h, sample_magic=True, sre_type=["virtual"], num_runs=1)
    submit_jobs(f"ising_test", param_bundle=param_matrix, ncores=64, memory="10gb", time="6:00:00", nodes=1, cleanup=False)

    param_matrix = generate_config(L, h, sample_magic=True, sre_type=["virtual"], num_runs=1)
    #submit_jobs(f"xxz_test", param_bundle=param_matrix, ncores=64, memory="10gb", time="6:00:00", nodes=1, cleanup=False)

    L = [256]
    h = 1.0
    param_matrix = generate_config(L, h, sample_magic=False, num_runs=1)
    #submit_jobs(f"ising_entanglement_test", param_bundle=param_matrix, ncores=4, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=True)
