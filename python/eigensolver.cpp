#include "../include/eigensolverapi/eigensolver.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace eigensolverapi {

PYBIND11_MODULE(eigensolverapi, m) {
    m.doc() = "Python bindings for the eigensolverapi module";
    // Add bindings for your C++ classes and functions here

    py::class_<System>(m, "System")
      .def(py::init<int>(), py::arg("n"))
      .def_readonly("eigenvalues", &System::eigenvalues)
      .def_readonly("eigenvectors", &System::eigenvectors)
      .def_readonly("uq_values", &System::uq_values)
      .def_readonly("uq_vectors", &System::uq_vectors)
      .def_readonly("n", &System::n);

    m.def("run_eigensolver", &run_eigensolver, py::arg("matrix_in"),
          "Find the eigenvalues and eigenvectors of a matrix.");
}

} // namespace eigensolverapi
