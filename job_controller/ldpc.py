from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np

def erdos_ldpc_config(system_size, pr, pb, single_site=True, include_isolated_in_core=True, nruns=1):
    config = {"circuit_type": "ldpc"}
    
    config["model_type"] = 0
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["pr"] = pr
    config["pb"] = pb

    config["single_site"] = single_site
    config["include_isolated_in_core"] = include_isolated_in_core

    config["sample_leaf_removal"] = True
    config["sample_rank"] = True
    config["sample_solveable"] = True

    config["max_size"] = 20
    
    return config
    

def regular_ldpc_config(system_size, pr, k, single_site=True, include_isolated_in_core=True, nruns=1):
    config = {"circuit_type": "ldpc"}
    
    config["model_type"] = 1
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["pr"] = pr
    config["k"] = k

    config["single_site"] = single_site
    config["include_isolated_in_core"] = include_isolated_in_core

    config["sample_leaf_removal"] = True
    config["sample_rank"] = True
    config["sample_solveable"] = True

    config["max_size"] = 20
    
    return config

def lattice_ldpc_config(system_size, pr, obc, single_site=True, include_isolated_in_core=True, model_type=2, nruns=1):
    config = {"circuit_type": "ldpc"}
    
    config["model_type"] = model_type
    config["num_runs"] = nruns
    
    config["system_size"] = system_size
    config["pr"] = pr
    config["obc"] = obc

    config["single_site"] = single_site
    config["include_isolated_in_core"] = include_isolated_in_core

    config["sample_leaf_removal"] = True
    config["sample_rank"] = True
    config["sample_solveable"] = True


    config["max_size"] = 20
    
    return config

ldpc_5body = 2
ldpc_3body = 3
ldpc_4body = 4

system_size = [8, 16, 32, 64, 128]
pr = list(np.linspace(0., 1.0, 20))
pb = list(np.linspace(0., 1.0, 20))
nruns = 50
config = erdos_ldpc_config(system_size=system_size, pr=pr, pb=pb, single_site=[0,1], include_isolated_in_core=[0,1], nruns=nruns)
#submit_jobs(config, f"ldpc_erdos", ncores=16, nodes=10, memory="20gb", time="5:00:00")

system_size = [8, 32, 64, 128]
p = list(np.linspace(0., 1.0, 20))
k = [3, 5]
config = regular_ldpc_config(system_size=system_size, pr=p, k=k, single_site=[0,1], include_isolated_in_core=[0,1], nruns=nruns)
#submit_jobs(config, f"ldpc_regular", ncores=16, nodes=10, memory="20gb", time="05:00:00")

system_size = [8, 16, 32, 64]
p = list(np.linspace(0.0, 1.0, 25))
config = lattice_ldpc_config(system_size=system_size, pr=p, obc=[False], single_site=[1], include_isolated_in_core=[0, 1], model_type=ldpc_4body, nruns=10)
submit_jobs(config, f"ldpc_lattice_4body", ncores=48, nodes=5, memory="50gb", time="24:00:00")
