from job_controller import config_to_string, submit_jobs, save_config

def vqse_circuit_config(num_qubits, hamiltonian_type=0, target_depth=1, maxiter=1000, nruns=16):
    config = {"circuit_type": "vqse_circuit"}
    
    # VQSE parameters
    config["num_runs"] = nruns
    config["num_qubits"] = num_qubits
    config["hamiltonian_type"] = hamiltonian_type
    config["num_iterations_per_layer"] = maxiter
    config["update_frequency"] = 20
    config["sampling_type"] = 0
    config["num_shots"] = 0
    
    config["gradient_type"] = 0
    config["noisy_gradients"] = False

    config["sampling_type"] = 0

    config["params_init"] = 0 # random

    # Target
    config["target_depth"] = target_depth
    config["mzr_prob"] = 1.0

    # Ansatz
    config["rotation_gates"] = "Rx, Ry, Rz"
    config["entangling_gate"] = "cz"
    
    # Data collection
    config["record_err"] = True
    config["record_fidelity"] = True
    
    return config_to_string(config)


config = vqse_circuit_config(8, nruns=16, maxiter=1000, target_depth=16, hamiltonian_type=2)
submit_jobs(config, "vqse_circuit", ncores=64, ncores_per_task=4, memory="10gb", time="24:00:00", cleanup=True)