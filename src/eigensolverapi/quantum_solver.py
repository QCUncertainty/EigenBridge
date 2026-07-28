import numpy as np
from qiskit.quantum_info import SparsePauliOp
from qiskit.circuit.library import EfficientSU2
from qiskit_algorithms import VQE
from qiskit_algorithms.optimizers import SLSQP
from qiskit.primitives import StatevectorEstimator

def solve_vqe(flat_matrix, n):
    """
    Takes a flat matrix of size n*n, 
    and returns the lowest eigenvalue using VQE.
    """
    # Reshape into an n x n NumPy array
    mat = np.array(flat_matrix).reshape((n, n))

    # Pad the matrix if it doesn't fit perfectly into a qubit register
    next_pow2 = 1 << (n - 1).bit_length()
    if next_pow2 != n:
        padded_mat = np.zeros((next_pow2, next_pow2))
        padded_mat[:n, :n] = mat
        
        # Add a massive penalty for the padded diagonal elements
        for i in range(n, next_pow2):
            padded_mat[i, i] = 1000.0 
        
        mat = padded_mat
        
    num_qubits = int(np.log2(next_pow2))

    
    observable = SparsePauliOp.from_operator(mat)

    # Setup the VQE environment 
    ansatz = EfficientSU2(num_qubits, reps=1, entanglement="linear")
    optimizer = SLSQP(maxiter=1000, ftol=1e-9)
    estimator = StatevectorEstimator()
    
    # Execute the VQE solver
    vqe = VQE(estimator, ansatz, optimizer)
    vqe.initial_point = np.zeros(ansatz.num_parameters)
    
    
    result = vqe.compute_minimum_eigenvalue(observable)

    return result.eigenvalue.real

# Quick Test
if __name__ == "__main__":
    # The same 3x3 matrix from the C++ test file
    test_matrix = [3.0, 5.0, 2.0, 5.0, 1.0, 3.0, 2.0, 3.0, 2.0]
    result = solve_vqe(test_matrix, 3)
    print(f"Quantum Minimum Eigenvalue: {result}")