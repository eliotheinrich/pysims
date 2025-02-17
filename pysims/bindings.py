#from dataframe import SimulatorConfig, CppConfig, ParallelCompute, DataFrame, DataSlide
#import pickle as pkl

import sys
import os

from pysims.pysimulators import *
#simulators = {
#    "quantum_automaton": QuantumAutomatonSimulator,
#    "random_clifford": RandomCliffordSimulator,
#    "soc_clifford": SelfOrganizedCliffordSimulator,
#    "sandpile_clifford": SandpileCliffordSimulator,
#    "mincut": MinCutSimulator,
#    "blocksim": BlockSimulator,
#    "rpm": RPMSimulator,
#    "graphsim": GraphCliffordSimulator,
#    "brickwork_circuit": BrickworkCircuitSimulator,
#    "partner": PartneringSimulator,
#    "groversat": GroverSATSimulator,
#    "phaseless": PhaselessSimulator,
#    "network_clifford": NetworkCliffordSimulator,
#    "env_sim": EnvironmentSimulator,
#    "bulk_sim": BulkMeasurementSimulator,
#    "random_circuit_sampling": RandomCircuitSamplingSimulator,
#    "random_hamiltonian": RandomHamiltonianSimulator,
#    "xz_circuit": XZCircuitSimulator,
#    "mps_simulator": MatrixProductSimulator,
#}
#
#config_types = {
#    "vqse": VQSEConfig,
#    "vqse_circuit": VQSECircuitConfig,
#    "magic_test": MagicTestConfig,
#    "quantum_ising": QuantumIsingTestConfig,
#    "hq_circuit": HQCircuitConfig,
#    "clifford_clustering": CliffordClusteringConfig,
#}
#
## Register external configs and simulators
#def register(module_name, simulators=None, configs=None):
#    if simulators is None:
#        simulators = {}
#
#    if configs is None:
#        configs = {}
#
#
#    simulator_names = ", ".join(list(simulators.values()) + list(configs.values()))
#    simulator_lines = "\n    ".join([f"simulators[\"{label}\"] = {name}" for label, name in simulators.items()])
#    config_lines = "\n    ".join([f"config_types[\"{label}\"] = {name}" for label, name in configs.items()])
#
#    code = [
#        f"try:",
#        f"    from {module_name} import {simulator_names}",
#        f"    {simulator_lines}",
#        f"    {config_lines}",
#        f"except ModuleNotFoundError:",
#        f"    pass",
#        f"except Exception as e:",
#        f"    raise e",
#    ]
#
#    code = "\n".join(code)
#
#    exec(code, globals())
#
#register("pyev", {"evolution": "EvolutionModel"})
#
#register("pyfe", {"duplicate_sim": "DuplicateSimulator"})
#
#pyxorsat_simulators = {
#    "clifford_code": "CliffordCodeSimulator",
#    "graph_clustering": "GraphClusteringSimulator"
#}
#
#pyxorsat_configs = {
#    "graph_xorsat": "GraphXORSATConfig",
#    "xorsat": "XORSATConfig",
#    "ldpc": "LDPCConfig",
#    "rxpm": "RXPMDualConfig",
#    "rpmca": "RPMCAConfig",
#    "slanted_checkerboard": "SlantedCheckerboardConfig"
#}
#
#register("pyxorsat", pyxorsat_simulators, pyxorsat_configs)
#
#register("pywsdc", None, {"wsdc_score": "ScoreSheetConfig"})
#
#pymc_simulators = {
#    "simple_graph": "SimpleGraphModel",
#    "square_ising": "SquareIsingModel",
#    "square_xy": "SquareXYModel",
#    "trigonal_xy": "TrigonalXYModel",
#    "trigonal_heis": "TrigonalModel",
#    "xxz_heis": "XXZHeis",
#    "ldpc_ising": "LDPCIsingModel"
#}
#
#register("pymc", pymc_simulators)
#
#pyneural_simulators = {
#    "noisy_neural": "NoisyNeuralSimulator",
#    "lattice_neural": "LatticeNeuralSimulator",
#    "nonlocal_neural": "NonlocalNeuralSimulator"
#}
#
#register("pyneural", pyneural_simulators)
#
#register("pyqca", {"qca": "QCASimulator"})
#
#sys.path.append("/data/heinriea/doped_clifford")
#register("doped_clifford", {"doped_clifford": "DopedCliffordSimulator"})
#
#
#def prepare_config(config_generator, params):
#    return config_generator(params)

#config_type = params["config_type"]
#    if config_type in simulators:
#        simulator_generator = simulators[config_type]
#        return SimulatorConfig(params, simulator_generator)
#    else:
#        return CppConfig(params, config_types[config_type])

