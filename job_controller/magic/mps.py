from dataframe import ZippedParams
import numpy as np

def generate_param_matrix(
        system_size, beta, p, bond_dimension=64, measurement_type=1, unitary_type=2,
        subsystem_fraction=8, sample_stabilizer_entropy=False, sample_stabilizer_entropy_mutual=False, sample_stabilizer_entropy_bipartite=False, sre_method="montecarlo", sre_num_samples=5000,
        sampling_timesteps=50, measurement_freq=5, equilibration_timesteps=30,
        **kwargs
    ):

    config = {}
    config["config_type"] = "mps_simulator"

    system_size = list(system_size)
    config["system_size"] = ZippedParams([{"system_size": L, "stabilizer_entropy_mutual_subsystem_size": L//subsystem_fraction} for L in system_size])
    config["bond_dimension"] = bond_dimension
    config["beta"] = beta
    config["p"] = p

    config["unitary_type"] = unitary_type
    config["measurement_type"] = measurement_type

    config["sample_stabilizer_entropy"] = sample_stabilizer_entropy
    config["sample_stabilizer_entropy_mutual"] = sample_stabilizer_entropy_mutual
    config["sample_stabilizer_entropy_bipartite"] = sample_stabilizer_entropy_bipartite 
    config["stabilizer_entropy_indices"] = "1,2"
    config["sre_method"] = sre_method
    config["sre_mc_equilibration_timesteps"] = 500
    config["sre_num_samples"] = sre_num_samples

    config["spin_glass_assume_symmetry"] = (unitary_type == 2)
    config["spin_glass_order_basis"] = "X"

    config["sample_surface"] = True

    config["temporal_avg"] = False
    config["equilibration_timesteps"] = equilibration_timesteps
    config["sampling_timesteps"] = sampling_timesteps
    config["measurement_freq"] = measurement_freq

    for key, val in kwargs.items():
        config[key] = val

    return config

def config_generator(params):
    from dataframe import SimulatorConfig
    from pysims import MatrixProductSimulator
    return SimulatorConfig(params, MatrixProductSimulator)
