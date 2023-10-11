// Clifford simulators
#include <QuantumAutomatonSimulator.h>
#include <RandomCliffordSimulator.h>
#include <SandpileCliffordSimulator.h>
#include <SelfOrganizedCliffordSimulator.h>
#include <MinCutSimulator.h>
#include <BlockSimulator.h>
#include <GraphCliffordSimulator.h>

// Full quantum simulators
#include <GroverProjectionSimulator.h>
#include <GroverSATSimulator.h>
#include <BrickworkCircuitSimulator.h>
#include <VQSEConfig.hpp>

// Misc
#include <PartneringSimulator.h>

#include <DataFrame.hpp>
#include <nlohmann/json.hpp>

std::shared_ptr<Config> assemble_config(Params &param) {
    std::string circuit_type = get<std::string>(param, "circuit_type");

    if      (circuit_type == "quantum_automaton") return prepare_timeconfig<QuantumAutomatonSimulator>(param);
    else if (circuit_type == "random_clifford") return prepare_timeconfig<RandomCliffordSimulator>(param);
    else if (circuit_type == "soc_clifford") return prepare_timeconfig<SelfOrganizedCliffordSimulator>(param);
    else if (circuit_type == "sandpile_clifford") return prepare_timeconfig<SandpileCliffordSimulator>(param);
    else if (circuit_type == "mincut") return prepare_timeconfig<MinCutSimulator>(param);
    else if (circuit_type == "blocksim") return prepare_timeconfig<BlockSimulator>(param);
    else if (circuit_type == "graphsim") return prepare_timeconfig<GraphCliffordSimulator>(param);
    else if (circuit_type == "grover_projection") return prepare_timeconfig<GroverProjectionSimulator>(param);
    else if (circuit_type == "brickwork_circuit") return prepare_timeconfig<BrickworkCircuitSimulator>(param);
    else if (circuit_type == "vqse") return std::make_shared<VQSEConfig>(param);
    else if (circuit_type == "partner") prepare_timeconfig<PartneringSimulator>(param);
    else if (circuit_type == "groversat") return prepare_timeconfig<GroverSATSimulator>(param);
    else {
        throw std::invalid_argument("Invalid circuit type.");
    }
}

ParallelCompute build_pc(Params& metaparams, std::vector<Params>& params) {
    std::vector<std::shared_ptr<Config>> configs;

    for (auto param : params)
        configs.push_back(assemble_config(param));

    return ParallelCompute(metaparams, configs);
}

// For deserialization
ParallelCompute build_pc(Params& metaparams, const DataFrame &frame) {
    std::vector<std::shared_ptr<Config>> configs;

    for (auto const &slide : frame.slides) {
        Params params = slide.params;

        uint32_t i = 0;
        std::string serialization_key = "serialization_" + std::to_string(i);
		std::vector<std::string> serialization_data;
        while (params.count(serialization_key)) {
			serialization_data.push_back(get<std::string>(params, serialization_key));
			params.erase(serialization_key);

            i++;
            serialization_key = "serialization_" + std::to_string(i);
        }

		for (auto const &s : serialization_data) {
			auto config = assemble_config(params)->deserialize(params, s);
			configs.push_back(config);
		}
    }

    return ParallelCompute(metaparams, configs);
}

ParallelCompute build_pc(Params& metaparams, const nlohmann::json &data) {
	if (data.contains("params") && data.contains("metadata") && data.contains("slides")) {
		DataFrame frame(data.dump());
		return build_pc(metaparams, frame);
	} else {
		auto params = load_json(data.dump());
		return build_pc(metaparams, params);
	}
}

ParallelCompute build_pc(Params& metaparams, const std::string &s) {
    nlohmann::json data = nlohmann::json::parse(s);
	return build_pc(metaparams, data);
}
