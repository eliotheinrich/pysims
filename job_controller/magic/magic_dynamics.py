from mps import generate_param_matrix, config_generator
from dataframe import *
from pysims import submit_jobs
import numpy as np

if __name__ == "__main__":
    sre_method = "virtual"
    num_nodes = 15*6
    L = [32, 64, 128, 256]

    for Li in L:
        param_matrix = generate_param_matrix(
            [Li], beta=[0.8], p=0.5, measurement_type=1, unitary_type=2, bond_dimension=64,
            sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method=sre_method, sre_num_samples=5000,
            sampling_timesteps=30, measurement_freq=1, equilibration_timesteps=0,
            sample_configurational_entropy=True,
        )
        ncores = 28
        #submit_jobs(f"md_08_{Li}", config_generator, param_bundle=param_matrix, nruns=ncores, ncores=ncores, memory="10gb", time="36:00:00", nodes=num_nodes, cleanup=False, run_local=False)

    num_nodes = 10

    L = [256]
    for Li in L:
        param_matrix = generate_param_matrix(
            [Li], beta=[1.2], p=0.5, measurement_type=1, unitary_type=2, bond_dimension=64,
            sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method=sre_method, sre_num_samples=5000,
            sampling_timesteps=30, measurement_freq=5, equilibration_timesteps=500,
            sample_configurational_entropy=True, num_configurational_entropy_samples=1000
        )
        ncores = 15
        submit_jobs(f"_md_12_{Li}_eq", config_generator, param_bundle=param_matrix, nruns=ncores, ncores=ncores, memory="10gb", time="72:00:00", nodes=num_nodes, cleanup=False, run_local=False)

    L = [8]
    bond_dimension = 6#4
    for Li in L:
        param_matrix = generate_param_matrix(
            [Li], beta=[0.8], p=0.5, measurement_type=1, unitary_type=2, bond_dimension=bond_dimension,
            sample_sre=True, sample_mi=False, sample_bipartite_mi=False, sre_method=sre_method, sre_num_samples=5000,
            sampling_timesteps=30, measurement_freq=5, equilibration_timesteps=500,
            sample_configurational_entropy=True, num_configurational_entropy_samples=1000
        )
        nruns = 15
        ncores = nruns
        

        # ------------------- #
        num_nodes = 1
        param_matrix["sampling_timesteps"] = 0
        param_matrix["equilibration_timesteps"] = 100
        def no_op(params):
            return params

        def do_samples(params):
            params["equilibration_timesteps"] = 0
            params["sampling_timesteps"] = 30

        callbacks = [no_op, do_samples]
        # ------------------- #

        submit_jobs(f"md_08_{Li}_eq", config_generator, param_bundle=param_matrix, checkpoint_callbacks=callbacks, nruns=nruns, ncores=ncores, memory="10gb", time="72:00:00", nodes=num_nodes, cleanup=False, run_local=True)
