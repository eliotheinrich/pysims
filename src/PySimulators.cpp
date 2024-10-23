#include "Models.h"
#include <Models/Magic/MatrixProductSimulator.hpp>
#include <PyDataFrame.hpp>

NB_MODULE(pysimulators, m) {
  INIT_CONFIG();

  // Clifford models
  EXPORT_SIMULATOR_DRIVER(RandomCliffordSimulator);
  EXPORT_SIMULATOR_DRIVER(QuantumAutomatonSimulator);
  EXPORT_SIMULATOR_DRIVER(SandpileCliffordSimulator);
  EXPORT_SIMULATOR_DRIVER(SelfOrganizedCliffordSimulator);
  EXPORT_SIMULATOR_DRIVER(PostSelectionCliffordSimulator);
  EXPORT_SIMULATOR_DRIVER(PhaselessSimulator);
  EXPORT_SIMULATOR_DRIVER(NetworkCliffordSimulator);
  EXPORT_SIMULATOR_DRIVER(EnvironmentSimulator);
  EXPORT_SIMULATOR_DRIVER(BulkMeasurementSimulator);
  EXPORT_SIMULATOR_DRIVER(MinCutSimulator);
  EXPORT_SIMULATOR_DRIVER(GraphCliffordSimulator);
  EXPORT_SIMULATOR_DRIVER(XZCircuitSimulator);
  EXPORT_CONFIG(CliffordClusteringConfig);

  // General quantum models
  EXPORT_SIMULATOR_DRIVER(RandomCircuitSamplingSimulator);
  EXPORT_SIMULATOR_DRIVER(RandomHamiltonianSimulator);
  EXPORT_SIMULATOR_DRIVER(GroverProjectionSimulator);
  EXPORT_SIMULATOR_DRIVER(GroverSATSimulator);
  EXPORT_SIMULATOR_DRIVER(BrickworkCircuitSimulator);
  EXPORT_CONFIG(VQSEConfig);
  EXPORT_CONFIG(VQSECircuitConfig);
  EXPORT_CONFIG(MagicTestConfig);

  EXPORT_CONFIG(QuantumIsingTestConfig);
  EXPORT_CONFIG(HalfSystemQuantumIsingConfig);
  EXPORT_SIMULATOR_DRIVER(MatrixProductSimulator);

  // Miscellaneous models
  EXPORT_SIMULATOR_DRIVER(PartneringSimulator);
  EXPORT_SIMULATOR_DRIVER(BlockSimulator);
  EXPORT_SIMULATOR_DRIVER(RPMSimulator);
  EXPORT_CONFIG(HQCircuitConfig);

  nanobind::class_<Graph<>>(m, "Graph")
    .def(nanobind::init<uint32_t>())
    .def(nanobind::init<>())
    .def("__str__", &Graph<>::to_string)
    .def("remove_edge", &Graph<>::remove_edge)
    .def("add_edge", &Graph<>::add_edge);
  

  m.def("graph_state_entropy", 
      [](Graph<> &graph, const std::vector<uint32_t>& sites) { 
        return QuantumGraphState::graph_state_entropy(sites, graph); 
      });

}
