from job_controller import config_to_string, submit_jobs, save_config
from pysims.pysimulators import *


def vqse_config(num_qubits):
    config = {"circuit_type": "vqse"}
    
    # VQSE parameters
    config["num_runs"] = 16
    config["num_qubits"] = num_qubits
    config["hamiltonian_type"] = [0]
    config["m"] = 2 
    config["num_iterations"] = 2000
    config["gradient_type"] = 0
    config["noisy_gradients"] = False
    #config["gradient_noise"] = [0.0, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1]

    config["simulated_sampling"] = False
    #config["num_shots"] = [1024, 2048, 4096, 8192, 16384, 32768]

    config["params_init"] = 1 # random

    # Target
    #config["target_type"] = 1 # real
    config["target_type"] = 0 # haar
    config["target_depth"] = num_qubits
    config["num_measurements"] = 1
    config["post_measurement_layers"] = 0
    config["density_matrix_target"] = True

    # Ansatz
    config["ansatz_depth"] = list(range(num_qubits//2, 2*num_qubits+1, num_qubits//2))
    config["rotation_gates"] = "Rx, Ry, Rz"
    config["entangling_gate"] = "cz"
    
    # Data collection
    config["record_fidelity"] = True
    
    return config_to_string(config)


config = vqse_config(6)
submit_jobs(config, "vqse_test6", ncores=64, memory="150gb", time="48:00:00")

def vqse_config_test(num_qubits):
    config = {"circuit_type": "vqse"}
    
    # VQSE parameters
    config["num_runs"] = 1
    config["num_qubits"] = num_qubits
    config["hamiltonian_type"] = [0, 1, 2]
    config["m"] = 2
    config["num_iterations"] = 1
    config["gradient_type"] = [0]
    config["noisy_gradients"] = True
    config["gradient_noise"] = [0.0, 1e-9, 1e-8, 1e-7, 1e-6, 1e-5, 1e-4, 1e-3, 1e-2, 1e-1]

    config["simulated_sampling"] = False
    #config["num_shots"] = [1024, 2048, 4096, 8192, 16384, 32768]

    # Target
    config["target_type"] = 1 # real
    config["target_depth"] = num_qubits
    config["num_measurements"] = 1
    config["post_measurement_layers"] = 2
    config["density_matrix_target"] = True

    # Ansatz
    config["ansatz_depth"] = num_qubits
    config["rotation_gates"] = "Ry, Rz"
    config["entangling_gate"] = "cz"
    
    # Data collection
    config["record_fidelity"] = True
    config["record_energy_levels"] = True
    config["num_energy_levels"] = 4
    
    return config_to_string(config)
