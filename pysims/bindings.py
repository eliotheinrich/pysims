from dataframe import TimeConfig

from pysims.pysimulators import *
simulators = {
    "quantum_automaton": QuantumAutomatonSimulator,
    "random_clifford": RandomCliffordSimulator,
    "soc_clifford": SelfOrganizedCliffordSimulator,
    "sandpile_clifford": SandpileCliffordSimulator,
    "mincut": MinCutSimulator,
    "blocksim": BlockSimulator,
    "rpm": RPMSimulator,
    "graphsim": GraphCliffordSimulator,
    "grover_projection": GroverProjectionSimulator,
    "brickwork_circuit": BrickworkCircuitSimulator,
    "partner": PartneringSimulator,
    "groversat": GroverSATSimulator,
    "phaseless": PhaselessSimulator,
    "network_clifford": NetworkCliffordSimulator,
    "env_sim": EnvironmentSimulator,
    "bulk_sim": BulkMeasurementSimulator,
    "random_circuit_sampling": RandomCircuitSamplingSimulator,
    "random_hamiltonian": RandomHamiltonianSimulator,
}

from pyev import EvolutionModel
simulators["evolution"] = EvolutionModel

from pyfe import DuplicateSimulator
simulators["duplicate_sim"] = DuplicateSimulator

config_types = {
    "vqse": VQSEConfig,
    "vqse_circuit": VQSECircuitConfig,
    "hq_circuit": HQCircuitConfig,
    "clifford_clustering": CliffordClusteringConfig,
}

from pyxorsat import XORSATConfig, GraphXORSATConfig, LDPCConfig, CliffordCodeSimulator, GraphClusteringSimulator, RXPMDualConfig, SlantedCheckerboardConfig
simulators["clifford_code"] = CliffordCodeSimulator
simulators["graph_clustering"] = GraphClusteringSimulator
config_types["graph_xorsat"] = GraphXORSATConfig
config_types["xorsat"] = XORSATConfig
config_types["ldpc"] = LDPCConfig
config_types["rxpm"] = RXPMDualConfig
config_types["slanted_checkerboard"] = SlantedCheckerboardConfig

from pywsdc import ScoreSheetConfig
config_types["wsdc_score"] = ScoreSheetConfig

from pymc import SimpleGraphModel, SquareIsingModel, SquareXYModel, TrigonalXYModel, XXZHeis, TrigonalModel, LDPCIsingModel
simulators["simple_graph"] = SimpleGraphModel
simulators["square_ising"] = SquareIsingModel
simulators["square_xy"] = SquareXYModel
simulators["trigonal_xy"] = TrigonalXYModel
simulators["trigonal_heis"] = TrigonalModel
simulators["xxz_heis"] = XXZHeis
simulators["ldpc_ising"] = LDPCIsingModel

def prepare_config(params):
    circuit_type = params["circuit_type"]
    if circuit_type in simulators:
        simulator_generator = simulators[circuit_type]
        return TimeConfig(params, simulator_generator)
    else:
        config_generator = config_types[circuit_type]
        return config_generator(params)

