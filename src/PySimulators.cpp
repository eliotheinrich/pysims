#include "utils.cpp"
#include <PyDataFrame.cpp>


#include <nanobind/nanobind.h>

namespace nb = nanobind;
using namespace nb::literals;



NB_MODULE(pysimulators, m) {
	init_dataframe(m);

    m.def("build_pc", static_cast<ParallelCompute(*)(Params&, const std::string&)>(&build_pc));
    m.def("build_pc", static_cast<ParallelCompute(*)(Params&, std::vector<Params>&)>(&build_pc));
    m.def("build_pc", static_cast<ParallelCompute(*)(Params&, const DataFrame&)>(&build_pc));
}