# RefineMesh

Refines an area in a mesh to have elements of smaller size.

**Detailed Description**

RefineMesh is meant to make a portion or all of the elements of a mesh a smaller size. The refinement effectively divides each element so that the edge lengths are half, yielding four in the case of hex elements and eight in the case of tet elements. This module works on both hex and tet elements but works best on structured meshes.

The inputs of RefineMesh are the field containing the mesh that you wish to refine and a matrix input for the isovalue used to define the refinement region.

The outputs are a field containing the refined mesh and a mapping matrix that maps data from the unrefined mesh to the refined one.

There are several options in the RefineMesh UI that control the refinement strategy and the region of refinement. The first parameter sets the refinement strategy to the default or an expanded region to improve quality. The second parameter defines the portion of the mesh to refine. There must be non-uniform data on the mesh to specify a region to refine. The options are to refine all (no contraint), refine less than isovalue, refine unequal to isovalue, refine greater than isovalue, or refine nothing. The isovalue can be set in the provided field. The most common way to set a refinement region is to create a distance map determine the desired distance to use. [CalculateDistanceToField](../ChangeFieldData/CalculateDistanceToField.md) can create a distance map to another mesh.

The RefineMesh algorithm expects the input field to contain data values. Data can be created on an input field using the [CreateFieldData](../ChangeFieldData/CreateFieldData) module.
