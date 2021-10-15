# ConvertMeshToUnstructuredMesh

Convert a structured mesh into an unstructured mesh for editing. An unstructured mesh is also implicitly irregular.

**Detailed Description**

This converts a LatVolField into a HexVolField, an ImageField into a QuadSurfField, or a ScanlineField into a CurveField. The structured meshes are not editable, as that operation does not preserve structure. Data values and locations are preserved in the transformation.
