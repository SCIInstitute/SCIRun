# ApplyFEMCurrentSource

The ApplyFEMCurrentSource module builds the right-hand side (ColumnMatrix) to reflect monopolar and dipolar current sources for finite-element based bioelectric field problems.

#### Inputs:

#### Outputs:

The first output, RHS, is the right-hand-side vector to be used with the stiffness matrix. The size of the vector is the number of nodes of the input mesh.

The second output, Weights, is a vector giving the weight of each component of the current source. When dipole sources are used, Weights contains the x/y/z-component of each dipole moments.

**Detailed Description**

If an input RHS ColumnMatrix is present, this module adds the contributions for the current sources into that matrix (i.e. multiple ApplyFEMCurrentSource modules can be cascaded together). If no RHS is present, this module will generate one. The RHS ColumnMatrix will be of length n, where n is the number of nodes in the finite element mesh.

We assume the mesh will be stored in a TetVolMesh, a HexVolMesh, or a TriSurfMesh.

There are two types of current sources supported by this module: dipoles, and sources-and-sinks.

With dipoles, the user must pass in a PointCloudField<Vector> into the Sources input port. The position of each PointCloud node corresponds to the location of the dipole, and the corresponding Vector corresponds to the moment of the dipole. If the user wishes to use dipolar sources, they must pass in a PointCloudField<Vector> AND they must select "dipole" for the Source Model on the UI window.

In contrast to the dipole model, the sources-and-sinks option has several sub-models. To activate any of these, the user must select the "sources and sinks" option from the UI. The values that are used for sources-and-sinks are a combination of: the source and sink indices entered in the UI; a PointCloudField<double> passed into the Source port; and a MappingMatrix. Here is the logic for how those values are used:
  1. if we don't have a Mapping matrix, we use the source/sink indices from the UI as node indices from the volume mesh between which one unit of current is passed;
  2. if we have a Mapping matrix, but we don't have a Source field, then the source/sink indices refer to the PointCloud and we use the Mapping matrix to get their corresponding volume node indices, and we then pass one unit of current between them;
  3. if we have a Mapping matrix AND a Source field, then ignore the source/sink indices from the UI, and assume that the Mapping matrix maps the PointCloud nodes to Volume mesh nodes and that the data values (doubles) from the PointCloud indicate the strength (current density) for each point source.
