from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def graph_clustering_config(system_size, graph_type, pz, pb=None, k=None, nruns=1, sampling_timesteps=1, equilibration_timesteps=0):
    config = {"circuit_type": "graph_clustering"}
    
    # VQSE parameters
    config["num_runs"] = nruns
    
    config["system_size"] = system_size 

    config["pz"] = pz

    config["graph_type"] = graph_type
    if pb is not None:
        config["pb"] = pb
    if k is not None:
        config["k"] = k

    config["temporal_avg"] = False
    config["equilibration_timesteps"] = equilibration_timesteps
    config["measurement_freq"] = 1
    config["sampling_timesteps"] = sampling_timesteps
    
    return config

config = graph_clustering_config(system_size=[16, 32, 64, 128, 256], graph_type=0, pz=list(np.linspace(0.0, 1.0, 20)), pb=list(np.linspace(0.0, 1.0, 10)), sampling_timesteps=1, equilibration_timesteps=0, nruns=5000)
submit_jobs(config, f"graph_clustering_erdos", ncores=8, nodes=20, memory="10gb", time="20:00:00", run_local=False)

config = graph_clustering_config(system_size=[16, 32, 64, 128, 256, 512, 1024], graph_type=1, pz=list(np.linspace(0.0, 1.0, 20)), k=[3,5], sampling_timesteps=1, equilibration_timesteps=0, nruns=5000)
submit_jobs(config, f"graph_clustering_regular", ncores=8, nodes=20, memory="10gb", time="20:00:00", run_local=False)