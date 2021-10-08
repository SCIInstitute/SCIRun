# ApplyFEMVoltageSource

The module changes/creates the right-hand side (RHS) and forward matrices for voltage (Dirichlet) boundary conditions.  

**Detailed Description**

The ApplyFEMVoltageSource module creates and fills a new ColumnMatrix, according to mesh and dipole parameters in situations where it does not locate a ColumnMatrix on its input that contains an RHS to modify, or when the module finds a ColumnMatrix of a different size than the number of nodes in the mesh.

<!-- TODO: Information out-of-date -->
<!-- We assume the mesh to be a TetVolField(int) type, containing indices to the conductivity tensor lookup table.
The documentation for the MODULE REFERENCE module provides further information about this mesh. -->
