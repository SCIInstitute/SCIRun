# GetCentroidsFromMesh

Computes a point cloud field containing all of the elements for a field.

**Detailed Description**

The Centroids module computes a point cloud field containing all the element /node/face/edge/cell/delement centers for a field. For instance, if the input field is a TetVolField then the output field would contain the center of each tetrahedra. An input point cloud field would return the same data locations.

The output field is always a point cloud field of doubles, and contains no data. If data is needed at those points then a mapping module ([MapFieldData](../ChangeFieldData/index.html) module) could be used to recover them.
