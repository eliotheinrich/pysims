import sys
import os
sys.path.insert(1, os.path.join(sys.path[0], '..'))

from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np


def lattice_ldpc_config(system_size, pr, obc, single_site=True, model_type=2, sample_bulk_symmetry=True, sample_bulk_mutual_information=True, nruns=1):
    config = {"circuit_type": "ldpc"}

    config["model_type"] = model_type
    config["num_runs"] = nruns

    config["system_size"] = system_size
    config["pr"] = pr
    config["obc"] = obc

    config["single_site"] = single_site

    config["sample_bulk_symmetry"] = sample_bulk_symmetry
    config["sample_bulk_mutual_information"] = sample_bulk_mutual_information
    config["sample_rank"] = False

    config["max_size"] = 20

    return config


ldpc_5body = 2
ldpc_3body = 3
ldpc_4body = 4

system_size = [32, 64]
p = list(np.linspace(0.0, 1.0, 25))
#p = []
p.append(0.743)
config = lattice_ldpc_config(system_size=system_size, pr=p, obc=False, single_site=1, model_type=[ldpc_5body], nruns=25)
submit_jobs(f"ldpc_lattice_5body", param_bundle=config, ncores=64, nodes=5, memory="50gb", time="24:00:00")

system_size = [32, 64]
p = list(np.linspace(0.0, 1.0, 25))
#p = []
p.append(0.810)
config = lattice_ldpc_config(system_size=system_size, pr=p, obc=False, single_site=1, model_type=[ldpc_3body], nruns=25)
submit_jobs(f"ldpc_lattice_3body", param_bundle=config, ncores=64, nodes=5, memory="50gb", time="24:00:00")

system_sizes = [32, 64, 128]
p = np.linspace(0.0, 1.0, 25)
config = lattice_ldpc_config(system_size=system_size, pr=p, obc=False, sample_bulk_symmetry=False, single_site=1, model_type=[ldpc_3body], nruns=500)
#submit_jobs(f"bulk_mi_3body", param_bundle=config, ncores=64, nodes=5, memory="50gb", time="24:00:00")

config = lattice_ldpc_config(system_size=system_size, pr=p, obc=False, sample_bulk_symmetry=False, single_site=1, model_type=[ldpc_4body], nruns=500)
#submit_jobs(f"bulk_mi_4body", param_bundle=config, ncores=64, nodes=5, memory="50gb", time="24:00:00")

config = lattice_ldpc_config(system_size=system_size, pr=p, obc=False, sample_bulk_symmetry=False, single_site=1, model_type=[ldpc_5body], nruns=500)
#submit_jobs(f"bulk_mi_5body", param_bundle=config, ncores=64, nodes=5, memory="50gb", time="24:00:00")
