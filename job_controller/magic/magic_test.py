import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, phi, state_type=0, num_t_gates=1, sre_type="montecarlo", apply_random_clifford=1, num_runs=1):
    config = {}
    config["circuit_type"] = "magic_test"
    config["num_runs"] = num_runs

    config["quantum_state_type"] = 1

    config["system_size"] = system_size
    config["phi"] = phi

    config["stabilizer_renyi_indices"] = "2"
    config["sample_stabilizer_renyi_entropy"] = True
    config["sre_method"] = sre_type
    config["sre_num_samples"] = 1000
    
    config["sample_probabilities"] = False
    config["sample_bitstring_distribution"] = False

    config["sample_magic_mutual_information"] = True

    config["state_type"] = state_type
    config["num_t_gates"] = num_t_gates
    config["apply_random_clifford"] = apply_random_clifford

    return config


if __name__ == "__main__":
    L = [8, 16, 32, 64]
    phi = np.linspace(0, np.pi/2, 50)
    phi_param_bundle = generate_config(L, phi, sre_type=["virtual", "montecarlo"], state_type=0, apply_random_clifford=[True], num_runs=1)
    #phi_param_bundle = generate_config(L, phi, sre_type=["virtual", "montecarlo", "exhaustive"], state_type=1, apply_random_clifford=[True], num_runs=1)
    submit_jobs(f"magic_test2", param_bundle=phi_param_bundle, ncores=4, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=True)

    L = 4
    t_doped_param_bundle = generate_config(L, 0.0, sre_type=["montecarlo", "virtual", "exhaustive"], state_type=2, num_t_gates=list(range(L)), apply_random_clifford=[True], num_runs=1)
    #submit_jobs(f"magic_test_L", param_bundle=t_doped_param_bundle, ncores=4, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=True)

    L = 4
    phi = np.linspace(0, np.pi, 100)
    haar_param_bundle = generate_config(L, phi, sre_type=["virtual"], state_type=1, num_runs=1)
    #submit_jobs(f"magic_test", param_bundle=haar_param_bundle, ncores=1, memory="10gb", time="6:00:00", nodes=1, cleanup=False, run_local=True)

