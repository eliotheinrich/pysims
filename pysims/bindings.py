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

config_types = {
    "vqse": VQSEConfig,
    "vqse_circuit": VQSECircuitConfig,
    "hq_circuit": HQCircuitConfig,
    "clifford_clustering": CliffordClusteringConfig,
}

# Register external configs and simulators
def register(module_name, simulators=None, configs=None):
    if simulators is None:
        simulators = {}

    if configs is None:
        configs = {}


    simulator_names = ", ".join(list(simulators.values()) + list(configs.values()))
    simulator_lines = "\n    ".join([f"simulators[\"{label}\"] = {name}" for label, name in simulators.items()])
    config_lines = "\n    ".join([f"config_types[\"{label}\"] = {name}" for label, name in configs.items()])

    code = [
        f"try:",
        f"    from {module_name} import {simulator_names}",
        f"    {simulator_lines}",
        f"    {config_lines}",
        f"except ModuleNotFoundError:",
        f"    pass",
        f"except Exception as e:",
        f"    raise e",
    ]

    code = "\n".join(code)

    exec(code, globals())

register("pyev", {"evolution": "EvolutionModel"})

register("pyfe", {"duplicate_sim": "DuplicateSimulator"})

pyxorsat_simulators = {
    "clifford_code": "CliffordCodeSimulator",
    "graph_clustering": "GraphClusteringSimulator"
}

pyxorsat_configs = {
    "graph_xorsat": "GraphXORSATConfig",
    "xorsat": "XORSATConfig",
    "ldpc": "LDPCConfig",
    "rxpm": "RXPMDualConfig",
    "rpmca": "RPMCAConfig",
    "slanted_checkerboard": "SlantedCheckerboardConfig"
}

register("pyxorsat", pyxorsat_simulators, pyxorsat_configs)

register("pywsdc", None, {"wsdc_score": "ScoreSheetConfig"})

pymc_simulators = {
    "simple_graph": "SimpleGraphModel",
    "square_ising": "SquareIsingModel",
    "square_xy": "SquareXYModel",
    "trigonal_xy": "TrigonalXYModel",
    "trigonal_heis": "TrigonalModel",
    "xxz_heis": "XXZHeis",
    "ldpc_ising": "LDPCIsingModel"
}

register("pymc", pymc_simulators)

pyneural_simulators = {
    "noisy_neural": "NoisyNeuralSimulator",
    "lattice_neural": "LatticeNeuralSimulator",
    "nonlocal_neural": "NonlocalNeuralSimulator"
}

register("pyneural", pyneural_simulators)

register("pyfermion", {"chain_fermion": "ChainSimulator", "adaptive_fermion": "AdaptiveFermionSimulator"})

def prepare_config(params, serialize):
    circuit_type = params["circuit_type"]
    if circuit_type in simulators:
        simulator_generator = simulators[circuit_type]
        return TimeConfig(params, simulator_generator, serialize)
    else:
        config_generator = config_types[circuit_type]
        return config_generator(params)

def resume_run(frame, callback=None, metaparams=None, serialize=False):
    if callback is None:
        def callback(x):
            return

    configs = []
    for slide in frame.slides:
        params = {**frame.params, **slide.params}
        config = prepare_config(params, serialize)
        callback(config.params)
        buffer = slide._get_buffer()
        config.store_serialized_simulator(buffer)
        configs.append(config)

    if metaparams is None:
        metaparams = frame.metadata
    pc = ParallelCompute(configs, **metaparams)

    new_frame = pc.compute()

    num_slides = len(frame.slides)
    for n in range(num_slides):
        new_frame.slides[n].params = frame.slides[n].params
        new_frame.slides[n].combine_data(frame.slides[n])
    return new_frame
