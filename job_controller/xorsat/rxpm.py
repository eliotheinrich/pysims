from job_controller import submit_jobs
from pysims.pysimulators import *

import numpy as np


def rxpm_config(system_size, p, nruns):
    config = {"circuit_type": "rxpm"}

    # VQSE parameters
    config["num_runs"] = nruns

    config["system_size"] = system_size
    config["p"] = p

    return config


system_sizes = [4, 6, 8, 10]
p = list(np.linspace(0.0, 1.0, 100))
nruns = 100
config = rxpm_config(system_sizes, p, nruns)
submit_jobs(config, f"rxdm_test", ncores=4, memory="5gb", time="2:00:00", run_local=True)
