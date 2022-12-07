# SolveInverseProblemWithTSVD

This module solves a linear inverse problem with a truncated singular value decomposition (TSVD).

**Detailed Description**

One can compute the inverse of an invertible matrix from its SVD. Specifically, an invertible matrix A=U*S*V' has an inverse inv(A)=V*inv(S)*U' (where "inv()" means matrix inverse). Since the matrix S is a diagonal matrix, its inverse is just the scalar inverse of its diagonal elements. However, when A is not invertible or it is ill-conditioned, a more stable solution to the inverse problem may be obtained by omitting the small or zero-valued singular values from the SVD-based matrix inverse computation, which is the method implemented in this module. This solution is mathematically equivalent to solving the inverse problem only in the subspace spanned by the right singular vectors (rows of V) corresponding to singular values that were not discarded.

This method is sometimes referred to as a pseudo-inverse when only the zero-valued singular values are discarded.
