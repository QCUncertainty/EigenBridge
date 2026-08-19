#include <catch2/catch_approx.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
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

TEST_CASE("Eigensolverapi vqd") {
    vector_t matrix      = {3, 5, 2, 5, 1, 3, 2, 3, 2};
    vector_t corr_values = {-3.3610452994996525, 0.5038738768058354,
                            8.8571714226938152};
    vector_t corr_vectors = {
      -0.5518254419285664, -0.5057445690691208, -0.6631071651682192,
      0.7984036023978691,  -0.0906811731252565, -0.5952550818924042,
      -0.2409156892326610, 0.8579040480716453,  -0.4538284642723896};
    const int n            = 3;
    const double vec_tol   = 5.0e-2;

    auto matches_column_up_to_sign = [](const vector_t& got,
                                        const vector_t& expected, int col,
                                        int order, double margin) {
        bool same    = true;
        bool flipped = true;
        for(int row = 0; row < order; ++row) {
            const double a = got[row * order + col];
            const double b = expected[row * order + col];
            if(std::abs(a - b) > margin) same = false;
            if(std::abs(a + b) > margin) flipped = false;
        }
        return same || flipped;
    };

    SECTION("default k equals n") {
        auto rv = eigensolverapi::run_vqd_eigensolver(matrix);
        for(std::size_t i = 0; i < corr_values.size(); ++i) {
            REQUIRE(rv.eigenvalues[i] ==
                    Catch::Approx(corr_values[i]).margin(1.0e-2));
        }
        for(int i = 0; i < n; ++i) {
            REQUIRE(matches_column_up_to_sign(rv.eigenvectors, corr_vectors, i,
                                              n, vec_tol));
        }
    }

    SECTION("k equals 1 returns only the ground state") {
        auto rv = eigensolverapi::run_vqd_eigensolver(matrix, 1);
        REQUIRE(rv.eigenvalues[0] ==
                Catch::Approx(corr_values[0]).margin(1.0e-2));
        REQUIRE(rv.eigenvalues[1] == 0.0);
        REQUIRE(rv.eigenvalues[2] == 0.0);
        REQUIRE(matches_column_up_to_sign(rv.eigenvectors, corr_vectors, 0, n,
                                          vec_tol));
        for(int col = 1; col < n; ++col) {
            for(int row = 0; row < n; ++row) {
                REQUIRE(rv.eigenvectors[row * n + col] == 0.0);
            }
        }
    }
}

TEST_CASE("Eigensolverapi qaoa") {
    vector_t matrix      = {3, 5, 2, 5, 1, 3, 2, 3, 2};
    vector_t corr_values = {-3.3610452994996525, 0.5038738768058354,
                            8.8571714226938152};
    vector_t corr_vectors = {
      -0.5518254419285664, -0.5057445690691208, -0.6631071651682192,
      0.7984036023978691,  -0.0906811731252565, -0.5952550818924042,
      -0.2409156892326610, 0.8579040480716453,  -0.4538284642723896};
    const int n          = 3;
    const double val_tol = 5.0e-2;
    const double vec_tol = 5.0e-2;

    auto matches_column_up_to_sign = [](const vector_t& got,
                                        const vector_t& expected, int col,
                                        int order, double margin) {
        bool same    = true;
        bool flipped = true;
        for(int row = 0; row < order; ++row) {
            const double a = got[row * order + col];
            const double b = expected[row * order + col];
            if(std::abs(a - b) > margin) same = false;
            if(std::abs(a + b) > margin) flipped = false;
        }
        return same || flipped;
    };

    auto rv = eigensolverapi::run_qaoa_eigensolver(matrix);
    REQUIRE(rv.eigenvalues[0] ==
            Catch::Approx(corr_values[0]).margin(val_tol));
    REQUIRE(rv.eigenvalues[1] == 0.0);
    REQUIRE(rv.eigenvalues[2] == 0.0);
    REQUIRE(matches_column_up_to_sign(rv.eigenvectors, corr_vectors, 0, n,
                                      vec_tol));
    for(int col = 1; col < n; ++col) {
        for(int row = 0; row < n; ++row) {
            REQUIRE(rv.eigenvectors[row * n + col] == 0.0);
        }
    }
    for(const auto& u : rv.uq_values) REQUIRE(u == 1e-16);
    for(const auto& u : rv.uq_vectors) REQUIRE(u == 1e-16);
}
