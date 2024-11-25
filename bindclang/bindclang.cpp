#include <Coverage/CoverageMapping.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace bindclang {

PYBIND11_MODULE(bindclang, m) {
  m.doc() = "Bind clang lib we needed.";

  pybind11::class_<Coverage::CoverageMapping>(m, "CoverageMapping")
      .def_static("LoadFromFile", &Coverage::CoverageMapping::LoadFromFile)
      .def("dump", &Coverage::CoverageMapping::dump)
      .def(
          "getCoveredFunctions",
          // TODO: 这里如果能够将这个抽象抽出去就好了
          [](const Coverage::CoverageMapping &cm) {
            auto rg = cm.getCoveredFunctions();
            return pybind11::make_iterator(rg.begin(), rg.end());
          },
          pybind11::keep_alive<0, 1>());

  pybind11::class_<llvm::coverage::FunctionRecord>(m, "FunctionRecord")
      .def("__repr__",
           [](const llvm::coverage::FunctionRecord &rec) {
             return "<FunctionRecord " + rec.Name + ">";
           })
      .def_readonly("Name", &llvm::coverage::FunctionRecord::Name)
      .def_readonly("Filenames", &llvm::coverage::FunctionRecord::Filenames)
      .def_readonly("CountedRegions",
                    &llvm::coverage::FunctionRecord::CountedRegions)
      .def_readonly("CountedBranchRegions",
                    &llvm::coverage::FunctionRecord::CountedBranchRegions);

  pybind11::class_<llvm::coverage::CountedRegion>(m, "CountedRegion")
      .def_readonly("ExecutionCount",
                    &llvm::coverage::CountedRegion::ExecutionCount)
      .def_readonly("FalseExecutionCount",
                    &llvm::coverage::CountedRegion::FalseExecutionCount)
      .def_readonly("Folded", &llvm::coverage::CountedRegion::Folded);
}

} // namespace bindclang
