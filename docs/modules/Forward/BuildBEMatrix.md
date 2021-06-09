# BuildBEMatrix

The module solves a discretized model of Laplace's equation in a 3D volume conductor model using "surface method", given a particular type of boundary condition.

**Detailed Description**

The specific problem the authors had in mind was the forward problem of electrocardiography which consists of calculating, for a given time instant, the potential distribution generated at the surface of a specified volume conductor due the presence of the selected equivalent sources on a surface inside the conductor (The module should work for other problems which fit the same physical description, but has only been tested for forward electrocardiography.) In surface methods, the different volume conductor regions are assumed to have a constant and isotropic conductivity, and only the interfaces between the different regions are triangulated and represented in the numerical model. This module uses the "Transfer-Coefficient Approach" or "solid-angle method" developed by Barr et al. [i], as extended to include torso inhomogeneities in [ii] and with an improved algorithm for computing the solid angles [iii].

[i] R.C. Barr, M. Ramsey, and M.S. Spach, "Relating epicardial to body surface potential distribution by means of transfer coefficients based on geometry measurement," IEEE Trans. Biomed. Eng., vol. BME-24, pp. 1-11, 1977.

[ii] P.C. Stanley, T.C. Pilkington, and M.N. Morrow., "The effects of thoracic inhomogeneities on the relationship between epicardial and torso potentials," IEEE Transactions on Biomedical Engineering, BME-33, pp.273-284, 1986.

[iii] J.A. De Munck, "Linear discretization of the volume conductor boundary integral equation using analytically integrated elements," IEEE Trans. Biomed. Eng., vol. 39, no. 9, 1992.

This module requires the triangulated surfaces of all the objects as inputs and creates the forward solution matrix as output. The geometric relationships of the surfaces are defined as described below, as are the boundary conditions to apply.

The number of input fields is two or greater and can be unlimited but one of them must be defined as the "source surface" where the Dirichlet boundary condition is given and another one defined as the "measurement surface" where the quantity of interest is to be calculated. To do this we use a "SetProperty" module for each of these two designated surfaces with `Property` = `in/out` and `Value` = `in` for the "source surface" and `Value` = `out` for the "measurement surface". Insulating boundary conditions (Neumann boundary conditions) are assumed on the outermost surface.

To define the geometric relationships of the various fields, for each of the input fields use a "SetProperty" module with `Property` = `Inside Conductivity` and `Value` = the numerical value of the internal conductivity of the corresponding homogeneous region.

The output is the forward solution matrix. This matrix can be multiplied to a Dirichlet boundary condition on the "source surface" to result in the boundary values on the "measurement surface". This matrix is needed as an input for the modules "TikhonovSVD", "Tikhonov" and "TSVD".
