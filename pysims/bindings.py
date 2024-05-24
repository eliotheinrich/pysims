from dataframe import TimeConfig, ParallelCompute, DataFrame, DataSlide
import pickle as pkl

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
    "xz_circuit": XZCircuitSimulator,
}

try:
    from pyev import EvolutionModel
    simulators["evolution"] = EvolutionModel
except ModuleNotFoundError:
    pass
except Exception as e:
    raise e

try:
    from pyfe import DuplicateSimulator
    simulators["duplicate_sim"] = DuplicateSimulator
except ModuleNotFoundError:
    pass
except Exception as e:
    raise e


config_types = {
    "vqse": VQSEConfig,
    "vqse_circuit": VQSECircuitConfig,
    "hq_circuit": HQCircuitConfig,
    "clifford_clustering": CliffordClusteringConfig,
}

try:
    from pyxorsat import XORSATConfig, GraphXORSATConfig, LDPCConfig, CliffordCodeSimulator, GraphClusteringSimulator, RXPMDualConfig, RPMCAConfig, SlantedCheckerboardConfig
    simulators["clifford_code"] = CliffordCodeSimulator
    simulators["graph_clustering"] = GraphClusteringSimulator
    config_types["graph_xorsat"] = GraphXORSATConfig
    config_types["xorsat"] = XORSATConfig
    config_types["ldpc"] = LDPCConfig
    config_types["rxpm"] = RXPMDualConfig
    config_types["rpmca"] = RPMCAConfig
    config_types["slanted_checkerboard"] = SlantedCheckerboardConfig
except ModuleNotFoundError:
    pass
except Exception as e:
    raise e

try:
    from pywsdc import ScoreSheetConfig
    config_types["wsdc_score"] = ScoreSheetConfig
except ModuleNotFoundError:
    pass
except Exception as e:
    raise e

try:
    from pymc import SimpleGraphModel, SquareIsingModel, SquareXYModel, TrigonalXYModel, XXZHeis, TrigonalModel, LDPCIsingModel
    simulators["simple_graph"] = SimpleGraphModel
    simulators["square_ising"] = SquareIsingModel
    simulators["square_xy"] = SquareXYModel
    simulators["trigonal_xy"] = TrigonalXYModel
    simulators["trigonal_heis"] = TrigonalModel
    simulators["xxz_heis"] = XXZHeis
    simulators["ldpc_ising"] = LDPCIsingModel
except ModuleNotFoundError:
    pass
except Exception as e:
    raise e


def prepare_config(params):
    circuit_type = params["circuit_type"]
    if circuit_type in simulators:
        simulator_generator = simulators[circuit_type]
        return TimeConfig(params, simulator_generator)
    else:
        config_generator = config_types[circuit_type]
        return config_generator(params)

def resume_run(frame, callback=None):
    if callback is None:
        def callback(x):
            return x

    configs = []
    for slide in frame.slides:
        params = {**frame.params, **slide.params}
        config = prepare_config(params)
        callback(config.params)
        config.store_serialized_simulator(slide._get_buffer())
        configs.append(config)

    pc = ParallelCompute(configs, **frame.metadata)
    return pc

