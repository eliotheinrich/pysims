#include "Models.h"
#include <Models/Magic/MatrixProductSimulator.hpp>
#include <PyQutils.hpp>

NB_MODULE(pysimulators, m) {
  // Clifford models
  EXPORT_SIMULATOR(RandomCliffordSimulator);
  EXPORT_SIMULATOR(QuantumAutomatonSimulator);
  EXPORT_SIMULATOR(SandpileCliffordSimulator);
  EXPORT_SIMULATOR(SelfOrganizedCliffordSimulator);
  EXPORT_SIMULATOR(PostSelectionCliffordSimulator);
  EXPORT_SIMULATOR(PhaselessSimulator);
  EXPORT_SIMULATOR(NetworkCliffordSimulator);
  EXPORT_SIMULATOR(EnvironmentSimulator);
  EXPORT_SIMULATOR(BulkMeasurementSimulator);
  EXPORT_SIMULATOR(MinCutSimulator);
  EXPORT_SIMULATOR(GraphCliffordSimulator);
  EXPORT_SIMULATOR(XZCircuitSimulator);
  EXPORT_CONFIG(CliffordClusteringConfig);

  // General quantum models
  EXPORT_SIMULATOR(RandomCircuitSamplingSimulator);
  EXPORT_SIMULATOR(RandomHamiltonianSimulator);
  EXPORT_SIMULATOR(GroverSATSimulator);
  EXPORT_SIMULATOR(BrickworkCircuitSimulator);
  EXPORT_CONFIG(VQSEConfig);
  EXPORT_CONFIG(VQSECircuitConfig);
  EXPORT_CONFIG(MagicTestConfig);

  EXPORT_CONFIG(QuantumIsingTestConfig);
  EXPORT_CONFIG(HalfSystemQuantumIsingConfig);
  EXPORT_SIMULATOR(MatrixProductSimulator);

  // Miscellaneous models
  EXPORT_SIMULATOR(PartneringSimulator);
  EXPORT_SIMULATOR(BlockSimulator);
  EXPORT_SIMULATOR(RPMSimulator);
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
