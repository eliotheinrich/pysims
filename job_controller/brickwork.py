from job_controller import config_to_string, submit_jobs, save_config
from pysims.pysimulators import *


def vqse_config(num_qubits):
    config = {"circuit_type": "brickwork_circuit"}
    
    config["num_runs"] = 4
    config["system_size"] = num_qubits

    config["mzr_prob"] = [0.2]
    
    config["equilibration_timesteps"] = 10
    config["sampling_timesteps"] = 10
    
    return config_to_string(config)


config = vqse_config(12)
submit_jobs(config, "test_brickwork", ncores=16, memory="150gb", time="1:00:00", run_local=True, ncores_per_task=4)

