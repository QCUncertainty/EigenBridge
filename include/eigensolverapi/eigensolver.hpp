#pragma once
#include <vector>

namespace eigensolverapi {

/** @brief Output from eigensolver. */
struct System {
public:
    using vector_t = std::vector<double>;

    System(int n) :
      n(n),
      eigenvalues(n),
      eigenvectors(n * n),
      uq_values(n),
      uq_vectors(n * n) {};

    // Pointer to the eigenvalues
    vector_t eigenvalues;

    // Pointer to the eigenvectors
    vector_t eigenvectors;

    // Pointer to the uncertainties of the eigenvalues
    vector_t uq_values;

    // Pointer to the uncertainties of the eigenvectors
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

} // namespace eigensolverapi
