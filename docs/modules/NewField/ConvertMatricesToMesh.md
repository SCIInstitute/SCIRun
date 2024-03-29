# ConvertMatricesToMesh

This module constructs a mesh from raw matrix data.

**Detailed Description**

ConvertMatricesToMesh takes in raw position and connectivity data as matrices and uses them to construct a new mesh. The 'Mesh Positions' data should be an Nx3 matrix where N is the number of points in the mesh and each row contains data in XYZ order. The connectivity data should be an MxP matrix where M is the number of elements in the mesh and P is the number of node references per element. Thus for a TetVolMesh, P would be 4. The node references are row indices into the 'Mesh Positions' matrix and are indexed starting at zero.

The output field will contain the new mesh and no data. Modules under the ChangeFieldData category, such as [CreateFieldData](../ChangeFieldData/CreateFieldData.md) or [SetFieldData](../ChangeFieldData/SetFieldData.md) module be used to attach data to the new field. See also modules in the ChangeMesh category, such as [SetFieldNodes](../ChangeMesh/SetFieldNodes.md), which can be used to attach new positional data to existing meshes but cannot construct a new mesh nor change it's connectivity information.
