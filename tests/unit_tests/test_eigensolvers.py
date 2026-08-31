# Copyright 2026 QHARM
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import unittest

import eigenbridge


class TestEigensolvers(unittest.TestCase):
    def setUp(self):
        # Define a 3x3 matrix
        self.flat_matrix = [3.0, 5.0, 2.0, 5.0, 1.0, 3.0, 2.0, 3.0, 2.0]
        # Correct results
        self.correct_eigenvalues = [
            -3.3610452994996525,
            0.5038738768058354,
            8.8571714226938152,
        ]
        self.correct_eigenvectors = [
            -0.5518254419285664,
            -0.5057445690691208,
            -0.6631071651682192,
            0.7984036023978691,
            -0.0906811731252565,
            -0.5952550818924042,
            -0.2409156892326610,
            0.8579040480716453,
            -0.4538284642723896,
        ]

    def _check_results(
        self,
        results,
        val_places=14,
        vec_places=14,
        uq_val=1e-16,
        uq_vec=1e-16,
    ):
        # Check the results
        eigenvalues, eigenvectors, uq_values, uq_vectors = results
        for i in zip(eigenvalues, self.correct_eigenvalues):
            self.assertAlmostEqual(i[0], i[1], places=val_places)
        for i in zip(eigenvectors, self.correct_eigenvectors):
            self.assertAlmostEqual(abs(i[0]), abs(i[1]), places=vec_places)
        for i in uq_values:
            self.assertEqual(i, uq_val)
        for i in uq_vectors:
            self.assertEqual(i, uq_vec)

    def test_vqd_eigensolver(self):
        results = eigenbridge.run_vqd_eigensolver(self.flat_matrix, n=3)
        self._check_results(
            results, val_places=8, vec_places=4, uq_val=0.0, uq_vec=0.0
        )

    def test_qaoa_eigensolver(self):
        # Adjust the correct eigenvalues and eigenvectors for QAOA's output
        # The QAOA solver will not return all eigenvalues/eigenvectors, so we
        # zero out the ones that are not expected to be returned.
        for i in range(len(self.correct_eigenvalues)):
            if i % 3 != 0:
                self.correct_eigenvalues[i] = 0.0
        for i in range(len(self.correct_eigenvectors)):
            if i % 3 != 0:
                self.correct_eigenvectors[i] = 0.0
        results = eigenbridge.run_qaoa_eigensolver(
            self.flat_matrix, n=3, use_noise=False
        )
        self._check_results(
            results, val_places=8, vec_places=4, uq_val=0.0, uq_vec=0.0
        )
