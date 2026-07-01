import eigensolverapi
import unittest


class TestEigensolver(unittest.TestCase):
    def test_eigensolver(self):
        # Define a 2x2 matrix
        flat_matrix = [3.0, 5.0, 2.0, 5.0, 1.0, 3.0, 2.0, 3.0, 2.0]

        # Call the eigensolver
        result = eigensolverapi.run_eigensolver(flat_matrix)

        # Correct results
        correct_eigenvalues = [
            -3.3610452994996525,
            0.5038738768058354,
            8.8571714226938152,
        ]
        correct_eigenvectors = [
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
        uncertainty = 1e-16

        # Check the results
        for i in range(len(result.eigenvalues)):
            self.assertAlmostEqual(
                result.eigenvalues[i], correct_eigenvalues[i], places=16
            )
        for i in range(len(result.eigenvectors)):
            self.assertAlmostEqual(
                result.eigenvectors[i], correct_eigenvectors[i], places=16
            )
        for i in result.uq_values:
            self.assertEqual(i, uncertainty)
        for i in result.uq_vectors:
            self.assertEqual(i, uncertainty)
