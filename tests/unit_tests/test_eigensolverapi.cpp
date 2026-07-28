#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <eigensolverapi/eigensolver.hpp>
#include <vector>

#include <iomanip>

using vector_t = std::vector<double>;

// TODO: Remove this
#include <iostream>

TEST_CASE("Eigensolverapi classical") {
    vector_t matrix       = {3, 5, 2, 5, 1, 3, 2, 3, 2};
    vector_t corr_values  = {-3.3610452994996525, 0.5038738768058354,
                             8.8571714226938152};
    vector_t corr_vectors = {
      -0.5518254419285664, -0.5057445690691208, -0.6631071651682192,
      0.7984036023978691,  -0.0906811731252565, -0.5952550818924042,
      -0.2409156892326610, 0.8579040480716453,  -0.4538284642723896};

    auto rv = eigensolverapi::run_eigensolver(matrix);
    for(std::size_t i = 0; i < rv.eigenvalues.size(); ++i) {
        REQUIRE(rv.eigenvalues[i] ==
                Catch::Approx(corr_values[i]).margin(1.0e-16));
    }
    for(std::size_t i = 0; i < rv.eigenvectors.size(); ++i) {
        REQUIRE(rv.eigenvectors[i] ==
                Catch::Approx(corr_vectors[i]).margin(1.0e-16));
    }
    for(const auto& u : rv.uq_values) REQUIRE(u == 1e-16);
    for(const auto& u : rv.uq_vectors) REQUIRE(u == 1e-16);
}

TEST_CASE("Eigensolverapi quantum") {
    vector_t matrix = {3, 5, 2, 5, 1, 3, 2, 3, 2};

    // VQE only computes the ground state energy
    // The classical ground state for this matrix is ~ -3.361
    double expected_ground_state = -3.3610452994996525;

    // Execute the embedded Python Qiskit script
    auto rv = eigensolverapi::run_quantum_eigensolver(matrix);

    // Check ONLY the ground state eigenvalue (index 0)
    // Note: Margin widened to 0.01 to account for VQE variance
    REQUIRE(rv.eigenvalues[0] ==
            Catch::Approx(expected_ground_state).margin(1.0e-2));
}
