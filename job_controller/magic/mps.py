import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from dataframe import *
from job_controller import submit_jobs
import numpy as np

def generate_config(system_size, beta, p, measurement_type=1, unitary_type=1, sample_sre=True, sre_method="montecarlo", num_runs=1, sampling_timesteps=50, measurement_freq=5, equilibration_timesteps=30):
    # Default to weak measurements and clifford unitaries
    config = {}
    config["circuit_type"] = "mps_simulator"
    config["num_runs"] = num_runs

    system_size = list(system_size)
    zparams = [{"system_size": L, "magic_mutual_information_subsystem_size": L//8} for L in system_size]
    config["bond_dimension"] = 64
    config["beta"] = beta
    config["p"] = p

    config["unitary_type"] = unitary_type
    config["measurement_type"] = measurement_type

    config["sample_magic_mutual_information"] = sample_sre
    config["sample_stabilizer_renyi_entropy"] = sample_sre
    config["sre_method"] = sre_method
    config["sre_mc_equilibration_timesteps"] = 500
    config["sre_num_samples"] = 5000
    config["zparams2"] = zparams

    config["sample_surface"] = True

    config["temporal_avg"] = False
    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = measurement_freq

    return config

if __name__ == "__main__":
    beta = 1.2

    sre_method = ["montecarlo"]
    num_runs = 2

    L = [8, 16, 32]
    p = np.linspace(0.0, 1.0, 24)
    param_matrix = generate_config(L, beta=beta, p=p, measurement_type=1, unitary_type=2, sre_method=sre_method, num_runs=num_runs)
    #submit_jobs(f"xxz_test_z2_1", param_bundle=param_matrix, ncores=48, memory="10gb", time="36:00:00", nodes=2, cleanup=False, run_local=False)

    L = [64]
    p = np.linspace(0.0, 1.0, 24)
    param_matrix = generate_config(L, beta=beta, p=p, measurement_type=1, unitary_type=2, sre_method=sre_method, num_runs=num_runs)
    #submit_jobs(f"xxz_test_z2_2", param_bundle=param_matrix, ncores=48, memory="10gb", time="36:00:00", nodes=2, cleanup=False, run_local=False)


    L = [8, 16, 32]
    p = np.linspace(0.0, 1.0, 24)
    param_matrix = generate_config(L, beta=beta, p=p, measurement_type=1, unitary_type=1, sre_method=sre_method, num_runs=num_runs)
    submit_jobs(f"xxz_test_1", param_bundle=param_matrix, ncores=48, memory="10gb", time="36:00:00", nodes=2, cleanup=False, run_local=False)

    L = [64]
    p = np.linspace(0.0, 1.0, 24)
    param_matrix = generate_config(L, beta=beta, p=p, measurement_type=1, unitary_type=1, sre_method=sre_method, num_runs=num_runs)
    #submit_jobs(f"xxz_test_2", param_bundle=param_matrix, ncores=48, memory="10gb", time="36:00:00", nodes=2, cleanup=False, run_local=False)


    #L = [64]
    #p = np.linspace(0.0, 1.0, 10)
    #param_matrix = generate_config(L, beta=beta, p=p, measurement_type=1, unitary_type=2, sample_sre=False, sre_method=["montecarlo"], equilibration_timesteps=0, sampling_timesteps=50, measurement_freq=1, num_runs=num_runs)
    #submit_jobs(f"xxz_test_64_t", param_bundle=param_matrix, ncores=48, memory="10gb", time="24:00:00", nodes=3, cleanup=False, run_local=False)

    #beta = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2]
    #L = [8, 16, 32]
    #param_matrix = generate_config(L, beta=beta, p=p, measurement_type=1, unitary_type=2, sample_sre=False, sre_method=["montecarlo"], equilibration_timesteps=0, sampling_timesteps=50, measurement_freq=1, num_runs=10)
    #submit_jobs(f"xxz_test_32_t", param_bundle=param_matrix, ncores=48, memory="10gb", time="24:00:00", nodes=3, cleanup=False, run_local=False)

