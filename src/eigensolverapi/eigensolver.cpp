#include "../../include/eigensolverapi/eigensolver.hpp"
#include <algorithm>
#include <cmath>
#include <lapacke.h>
#include <string>

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

} // namespace eigensolverapi
