#include <algorithm>
#include <cmath>
#include <lapacke.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>
#include <vector>

namespace py = pybind11;

namespace eigenbridge {

using vdouble = std::vector<double>;
using rv_t    = std::tuple<vdouble, vdouble, vdouble, vdouble>;

/** @brief Find the eigenvalues and eigenvectors of a matrix.
 *
 *  @param[in] matrix_in The matrix as a vector
 *  @param[in] n The order of the matrix
 *
 *  @return A System instance containing the eigenvalue, eigenvectors, and their
 *          respective uncertainties
 *
 *  @throw none No throw guarantee
 */
rv_t run_lapack_eigensolver(std::vector<double> matrix_in) {
    int n = std::sqrt(matrix_in.size());
    vdouble eigenvalues(n, 0.0);
    vdouble eigenvectors(n * n, 0.0);
    vdouble uq_values(n, 0.0);
    vdouble uq_vectors(n * n, 0.0);

    // LAPACK writes over the input values with the eigenvectors, so we copy the
    // input into the eigenvectors to start off.
    eigenvectors = matrix_in;

    int info = LAPACKE_dsyev(LAPACK_ROW_MAJOR, 'V', 'U', n, eigenvectors.data(),
                             n, eigenvalues.data());
    if(info > 0) {
        std::string msg = "eigensolverapi::run_eigensolver: The algorithm "
                          "failed to compute eigenvalues.";
        throw std::runtime_error(msg);
    }
    // For LAPACK, we currently just set the uncertainties to 1e-16.
    std::fill(uq_values.begin(), uq_values.end(), 1e-16);
    std::fill(uq_vectors.begin(), uq_vectors.end(), 1e-16);
    return rv_t(eigenvalues, eigenvectors, uq_values, uq_vectors);
}

PYBIND11_MODULE(eigenbridge_core, m) {
    m.doc() = "Python bindings for the eigenbridge module";
    m.def("run_lapack_eigensolver", &run_lapack_eigensolver,
          py::arg("matrix_in"),
          "Find the eigenvalues and eigenvectors of a matrix.");
}

} // namespace eigenbridge
