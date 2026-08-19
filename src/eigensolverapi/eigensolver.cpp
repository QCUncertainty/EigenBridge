#include "../../include/eigensolverapi/eigensolver.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <lapacke.h>
#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <stdexcept>
#include <string>

namespace py = pybind11;

namespace eigensolverapi {

System run_eigensolver(std::vector<double> matrix_in) {
    // Make system based on order of matrix
    int n = std::sqrt(matrix_in.size());
    System rv(n);

    // LAPACK writes over the input values with the eigenvectors, so we copy the
    // input into the eigenvectors to start off.
    rv.eigenvectors = matrix_in;

    int info = LAPACKE_dsyev(LAPACK_ROW_MAJOR, 'V', 'U', n,
                             rv.eigenvectors.data(), n, rv.eigenvalues.data());
    if(info > 0) {
        std::string msg = "eigensolverapi::run_eigensolver: The algorithm "
                          "failed to compute eigenvalues.";
        throw std::runtime_error(msg);
    }
    // For LAPACK, we currently just set the uncertainties to 1e-16.
    std::fill(rv.uq_values.begin(), rv.uq_values.end(), 1e-16);
    std::fill(rv.uq_vectors.begin(), rv.uq_vectors.end(), 1e-16);
    return rv;
}

System run_vqd_eigensolver(std::vector<double> matrix_in, int k) {
    int n = std::sqrt(matrix_in.size());
    if(k == -1) { k = n; }
    if(k < 1 || k > n) {
        throw std::runtime_error("eigensolverapi::run_vqd_eigensolver: k "
                                 "must satisfy 1 <= k <= n.");
    }
    System rv(n);

    // pybind11 allows only one interpreter per process
    if(!Py_IsInitialized()) { py::initialize_interpreter(); }

    try {
        py::object sys = py::module_::import("sys");
        sys.attr("path").attr("append")("../src/eigensolverapi");

        // Import the Python script
        py::object my_module = py::module_::import("quantum_solver");

        // Call solve_vqd; remaining eigenvalue/vector slots stay 0.0
        py::object result = my_module.attr("solve_vqd")(matrix_in, n, k);
        auto unpacked =
          result.cast<std::pair<std::vector<double>, std::vector<double>>>();
        auto& eigenvalues  = unpacked.first;
        auto& eigenvectors = unpacked.second;

        if(static_cast<int>(eigenvalues.size()) < k) {
            throw std::runtime_error(
              "eigensolverapi::run_vqd_eigensolver: VQD returned fewer "
              "eigenvalues than requested.");
        }
        if(static_cast<int>(eigenvectors.size()) < n * n) {
            throw std::runtime_error(
              "eigensolverapi::run_vqd_eigensolver: VQD returned fewer "
              "eigenvector components than n*n.");
        }
        for(int i = 0; i < k; ++i) { rv.eigenvalues[i] = eigenvalues[i]; }

        // Unused columns are already 0.0 from Python. This matches LAPACK
        // dsyev.
        std::copy(eigenvectors.begin(), eigenvectors.begin() + n * n,
                  rv.eigenvectors.begin());

        // Fill uncertainties with the default 1e-16 as required by the previous
        // system.
        std::fill(rv.uq_values.begin(), rv.uq_values.end(), 1e-16);
        std::fill(rv.uq_vectors.begin(), rv.uq_vectors.end(), 1e-16);

    } catch(py::error_already_set& e) {
        std::cerr << "Python execution failed: " << e.what() << std::endl;
        throw std::runtime_error("VQD solver encountered an error.");
    }

    return rv;
}

System run_qaoa_eigensolver(std::vector<double> matrix_in, bool use_noise) {
    int n = std::sqrt(matrix_in.size());
    System rv(n);

    if(!Py_IsInitialized()) { py::initialize_interpreter(); }

    try {
        py::object sys = py::module_::import("sys");
        sys.attr("path").attr("append")("../src/eigensolverapi");

        py::object my_module = py::module_::import("quantum_solver");
        py::object result =
          my_module.attr("solve_qaoa")(matrix_in, n, use_noise);
        auto unpacked =
          result.cast<std::pair<std::vector<double>, std::vector<double>>>();
        auto& eigenvalues  = unpacked.first;
        auto& eigenvectors = unpacked.second;

        if(eigenvalues.empty()) {
            throw std::runtime_error(
              "eigensolverapi::run_qaoa_eigensolver: QAOA returned no "
              "eigenvalue.");
        }
        if(static_cast<int>(eigenvectors.size()) < n * n) {
            throw std::runtime_error(
              "eigensolverapi::run_qaoa_eigensolver: QAOA returned fewer "
              "eigenvector components than n*n.");
        }
        rv.eigenvalues[0] = eigenvalues[0];
        std::copy(eigenvectors.begin(), eigenvectors.begin() + n * n,
                  rv.eigenvectors.begin());

        std::fill(rv.uq_values.begin(), rv.uq_values.end(), 1e-16);
        std::fill(rv.uq_vectors.begin(), rv.uq_vectors.end(), 1e-16);

    } catch(py::error_already_set& e) {
        std::cerr << "Python execution failed: " << e.what() << std::endl;
        throw std::runtime_error("QAOA solver encountered an error.");
    }

    return rv;
}

} // namespace eigensolverapi
