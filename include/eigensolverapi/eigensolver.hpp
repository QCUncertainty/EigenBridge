#pragma once
#include <vector>

namespace eigensolverapi {

/** @brief Output from eigensolver. */
struct System {
public:
    using vector_t = std::vector<double>;

    System(int n) :
      eigenvalues(n, 0.0),
      eigenvectors(n * n, 0.0),
      uq_values(n, 0.0),
      uq_vectors(n * n, 0.0),
      n(n) {};

    // Vector of the eigenvalues
    vector_t eigenvalues;

    // Vector of the eigenvectors
    vector_t eigenvectors;

    // Vector of the uncertainties of the eigenvalues
    vector_t uq_values;

    // Vector of the uncertainties of the eigenvectors
    vector_t uq_vectors;

    // The number of return values
    int n;
};

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
System run_eigensolver(std::vector<double> matrix_in);
System run_quantum_eigensolver(std::vector<double> matrix_in);

} // namespace eigensolverapi
