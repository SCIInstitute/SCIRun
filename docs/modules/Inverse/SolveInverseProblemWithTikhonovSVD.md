# SolveInverseProblemWithTikhonovSVD

<script type="text/javascript" async
  src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS_CHTML">
</script>

This module solves a linear inverse problem with Tikhonov regularization. It uses the singular value decomposition (SVD) or generalized SVD (GSVD) to compute its result.

**Detailed Description**

This module computes Tikhonov regularization as in SolveInverseProblemWithTikhonov but in a computationally efficient manner for multiple repetitions are needed since it uses the SVD (or GSVD if there is a regularization matrix) decomposition of the forward matrix. Thus, the optimization problem being solved is:

\\[ \hat{x}=argmin_x \|Ax - y\|^{2}_ {2} + \lambda\|x\|^{2}_ {2} \\]


Where the forward matrix <math>A</math> is expected to be given decomposed in its SVD terms that can be obtained with the module CIBC:Documentation:SCIRun:Reference:SCIRun:ComputeSVD. Note that computeSVD module returns the right singular vectors in rows and not in columns as it is required for SolveInverseProblemWithTikhonovSVD.

The GSVD should be used when the regularization matrix is different than idenity matrix (e.g. approximate Laplacian operator on a meshed volume or surface).


The solution is dependent on a scalar regularization parameter \\(\lambda\\) that can be specified directly using the User Interface of this module or can be determined automatically using the L-curve method. The automatic determination of the regularization parameter with the L-curve method requires computing several solutions to the inverse problem for a range of regularization parameters.


## Input

  1. Left Singular Vectors matrix, \\(U\\).

  2. Singular Values vector, \\(S\\).

  3. Right Singular Vectors matrix (vectors in columns), \\(V\\).

  4. Measured ECG vector, \\(y\\)

IMPORTANT NOTE: unlike some other software packages, this module expects the right singular matrix to contain its singular vectors in the columns. That is the matrix \\(V\\) in the decomposition \\( A=U* S *V^T \\).


## Output

  1. Inverse solution, \\(\hat{x}\\): computed solution estimate.

  2. Regularization parameter, \\( \lambda \\): used regularization parameter.

  3. Regularized inverse matrix, <math>G</math>: linear inverse operator that gives a solution estimate equation \\(\hat{x} = G y\\). It actual value depends on the selected formulation (underdetermined or overdetermined) and requires the inversion of a matrix. It is only calculated if this port is connected to another module's input port. The user can select between the formulations using the module GUI (see section Computation).
