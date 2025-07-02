from mps import generate_param_matrix, config_generator
from dataframe import *
from pysims import submit_jobs
import numpy as np

if __name__ == "__main__":
    num_nodes = 10
    ncores = 30
    
    beta = [0.0, 0.02, 0.04, 0.06, 0.08, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.8, 1.0, 1.5, 2.0, 3.0]
    L = [12, 14, 16, 18]

    for Li in L:
        param_matrix = generate_param_matrix(
            system_size=[Li], beta=beta, p=[0.2, 0.5, 0.8], measurement_type=1, unitary_type=2,
            sampling_timesteps=100, measurement_freq=5, equilibration_timesteps=[300], temporal_avg=True,
            sre_method="montecarlo",
            sample_stabilizer_entropy=False, sample_stabilizer_entropy_mutual=False, sample_stabilizer_entropy_bipartite=False, sre_num_samples=5000,
            participation_entropy_method="exhaustive",
            sample_participation_entropy=True, sample_participation_entropy_mutual=True, sample_participation_entropy_bipartite=False, num_participation_entropy_samples=5000,
            state_type=1,
        )
        submit_jobs(f"vol_transition_{Li}_300", config_generator, param_bundle=param_matrix, nruns=ncores, ncores=ncores, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False)

    #beta = [0.2, 0.4, 0.5, 0.6, 0.8]
    #L = [18, 20]

    #for Li in L:
    #    param_matrix = generate_param_matrix(
    #        system_size=[Li], beta=beta, p=[0.5], measurement_type=1, unitary_type=2,
    #        sampling_timesteps=100, measurement_freq=5, equilibration_timesteps=100, temporal_avg=True,
    #        state_type=1,
    #    )
    #    submit_jobs(f"volume_{Li}", config_generator, param_bundle=param_matrix, nruns=ncores, ncores=ncores, memory="10gb", time="12:00:00", nodes=num_nodes, cleanup=False)
