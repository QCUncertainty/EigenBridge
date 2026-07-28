# pip install "qiskit-nature[pyscf]"
# pip install qiskit-aer qiskit-ibm-runtime


import numpy as np
from qiskit_nature.second_q.drivers import PySCFDriver
from qiskit_nature.second_q.transformers import FreezeCoreTransformer
from qiskit_nature.second_q.mappers import ParityMapper
from qiskit_nature.second_q.algorithms import GroundStateEigensolver
from qiskit_algorithms import NumPyMinimumEigensolver
from qiskit_algorithms.minimum_eigensolvers import VQE
from qiskit_algorithms.optimizers import SLSQP
from qiskit_nature.second_q.circuit.library import HartreeFock, UCCSD
from qiskit.primitives import StatevectorEstimator
from qiskit_aer.primitives import EstimatorV2 as AerEstimatorV2   
from qiskit_aer.noise import NoiseModel
from qiskit_aer import AerSimulator
from qiskit_ibm_runtime.fake_provider import FakeManilaV2
from qiskit.transpiler.preset_passmanagers import generate_preset_pass_manager

# h20 molecule
driver = PySCFDriver(
    atom='O 0.0 0.0 0.0; H 0.757 0.586 0.0; H -0.757 0.586 0.0', charge=0, spin=0, basis='sto3g')
problem = driver.run()

# mapping the problem + classical calculation
transformer = FreezeCoreTransformer()
transformed_problem = transformer.transform(problem)
mapper = ParityMapper(num_particles=transformed_problem.num_particles)
tapered_mapper = transformed_problem.get_tapered_mapper(mapper)

algo = NumPyMinimumEigensolver()
algo.filter_criterion = transformed_problem.get_default_filter_criterion()
numpy_solver = GroundStateEigensolver(mapper, algo)
numpy_result = numpy_solver.solve(transformed_problem)
print("CLASSICAL RESULT")
print(numpy_result)
print("\n")

# Ansatz
initial_state = HartreeFock(
    transformed_problem.num_spatial_orbitals,
    transformed_problem.num_particles,
    tapered_mapper,
)
ansatz = UCCSD(
    transformed_problem.num_spatial_orbitals,
    transformed_problem.num_particles,
    tapered_mapper,
    initial_state=initial_state
)
 
fake_backend = FakeManilaV2()

# VQE
def print_progress(eval_count, parameters, mean, std):
    print(f"Iteration {eval_count}: Energy = {mean:.6f} Ha")

optimizer = SLSQP(maxiter=10, ftol=1e-9)

# SIMPLE (WITHOUT NOISE SIMULATION)

estimator = StatevectorEstimator()
vqe = VQE(estimator, ansatz, optimizer, callback=print_progress)
vqe.initial_point = [0] * ansatz.num_parameters
solver = GroundStateEigensolver(tapered_mapper, vqe)
result = solver.solve(transformed_problem)
print("FINAL RESULT")
print(result)

# WITH NOISE SIMULATION

noise_model = NoiseModel.from_backend(fake_backend)
aer_simulator_noisy = AerSimulator(noise_model=noise_model)
pass_manager = generate_preset_pass_manager(optimization_level=1, backend=aer_simulator_noisy)
noisy_estimator = AerEstimatorV2(
    options={"backend_options": {"noise_model": noise_model}}
)
vqe_noisy = VQE(
    noisy_estimator, ansatz, optimizer,
    callback=print_progress,
    transpiler=pass_manager,
)
vqe_noisy.initial_point = [0] * ansatz.num_parameters
solver_noisy = GroundStateEigensolver(tapered_mapper, vqe_noisy)
result_noisy = solver_noisy.solve(transformed_problem)
print("FINAL RESULT (with noise simulation)")
print(result_noisy)