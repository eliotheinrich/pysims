from job_controller import config_to_string, submit_jobs
import numpy as np

def ising_config(system_size, magnetic_field, temperature, equilibration_timesteps=5000, sampling_timesteps=5000, temporal_avg=True, nruns=1):
    config = {}
    config["circuit_type"] = "square_ising"
    config["num_runs"] = nruns

    config["system_size"] = system_size
    config["layers"] = 1
    config["J"] = 1.0
    config["B"] = magnetic_field
    config["zparams"] = [{"temperature": T, "initial_temperature": 2*T} for T in temperature]
    
    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["temporal_avg"] = temporal_avg

    return config
    

config = ising_config(
    system_size=[64], 
    magnetic_field=0.0, 
    temperature=np.linspace(1.8, 2.4, 20), 
    equilibration_timesteps=5000, 
    sampling_timesteps=10000, 
    temporal_avg=False, 
    nruns=5
)
#submit_jobs(config, f"ising1", ncores=16, nodes=1, memory="10gb", time="1:00:00", run_local=False, average_congruent_runs=False)

config = ising_config(
    system_size=[64], 
    magnetic_field=list(np.linspace(-0.02, 0.02, 20)), 
    temperature=[1.0], 
    equilibration_timesteps=5000, 
    sampling_timesteps=1000, 
    temporal_avg=False, 
    nruns=500
)
#submit_jobs(config, f"ising2", ncores=16, nodes=1, memory="10gb", time="1:00:00", run_local=False, average_congruent_runs=False)

def ldpc_ising_config(system_size, temperature, impurity, J=1.0, equilibration_timesteps=5000, sampling_timesteps=5000, temporal_avg=True, nruns=1):
    config = {}
    config["circuit_type"] = "ldpc_ising"
    config["num_runs"] = nruns

    config["system_size"] = system_size
    config["J"] = J
    config["impurity"] = impurity
    config["zparams"] = [{"temperature": T, "initial_temperature": 2*T} for T in temperature]
    
    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["temporal_avg"] = temporal_avg

    return config

config = ldpc_ising_config(
    system_size=[16, 32, 64], 
    J=[1.0, -1.0],
    temperature=[1.0],
    impurity=list(np.linspace(0.0, 1.0, 40)),
    equilibration_timesteps=50000, 
    sampling_timesteps=10000, 
    temporal_avg=True, 
    nruns=5
)
submit_jobs(config, f"ldpc_ising", ncores=48, nodes=5, memory="1gb", time="2:00:00", cleanup=False)
