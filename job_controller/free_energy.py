from job_controller import config_to_string, submit_jobs, save_config
import numpy as np

def duplication_config(
        system_size=[128], 
        K=1.0, 
        nruns=500, 
        graph_type=0, 
        alpha=2.0, 
        sampling_timesteps=None, 
        equilibration_timesteps=None,
        temperature=1.0,
        initial_temperature=2.0,
        **kwargs
    ):
    
    config = {}
    config["circuit_type"] = "duplicate_sim"
    config["num_runs"] = nruns
    
    config["augmented"] = [True, False]

    config["K"] = K
    config["graph_type"] = graph_type 
    config["alpha"] = alpha
    
    if not hasattr(system_size, "__iter__"):
        system_size = [system_size]

    zparams = []
    for L in system_size:
        params = {
            "system_size": L, 
            "subsystem_size": int(np.sqrt(L))
        }
            
        zparams.append(params)

    config["zparams"] = zparams


    # Monte Carlo settings
    config["temperature"] = temperature
    config["initial_temperature"] = initial_temperature
    config["cooling_schedule"] = "trig"

    config["temporal_avg"] = True
    config["sampling_timesteps"] = 3*max(system_size) if sampling_timesteps is None else sampling_timesteps
    config["equilibration_timesteps"] = 3*max(system_size) if equilibration_timesteps is None else equilibration_timesteps
    config["measurement_freq"] = 1

    return config

def modified_duplication_config(**kwargs):
    config = duplication_config(**kwargs)
    
    del config["zparams"]

    N = kwargs["system_size"]
    config["system_size"] = N

    if "subsystem_size" in kwargs:
        config["subsystem_size"] = kwargs["subsystem_size"]
    else:
        L = int(np.sqrt(N))
        config["subsystem_size"] = list(range(1, L))
    
    return config

if __name__ == "__main__":
    system_size = [16, 64, 256, 1024]
    K = list(-np.linspace(-0.8, 0.8, 50))
    config = duplication_config(system_size=system_size, K=K, graph_type=2, nruns=50)
    #submit_jobs(config, f"dupl_b", ncores=64, nodes=4, memory="50gb", time="4:00:00", run_local=False)

    system_sizes = [1024]
    K = list(-np.linspace(0.1, 0.8, 50))
    
    for L in system_size:
        config = modified_duplication_config(system_size=L, K=K, graph_type=2, nruns=500)
        #submit_jobs(config, f"dupl2d_{L}", ncores=48, nodes=2, memory="50gb", time="20:00:00", run_local=False)

    system_size = [16, 64, 256, 1024]
    K = list(-np.linspace(0.1, 0.8, 50))
    
    for L in system_size:
        Ls = int(np.sqrt(L))
        subsystem_size = list(range(Ls, L, Ls))
        config = modified_duplication_config(system_size=L, subsystem_size=subsystem_size, K=K, graph_type=2, nruns=500)
        submit_jobs(config, f"dupl2d_{L}", ncores=64, nodes=4, memory="50gb", time="20:00:00", run_local=False)