# BuildMappingMatrix

Build a mapping matrix; a matrix that says how to project the data from one field onto the data of a second field.

**Detailed Description**

This module builds a mapping matrix which contains information about how to interpolate the data from the source field onto the geometry of the destination field. The resulting mapping matrix can then be used by the [ApplyMappingMatrix](../ChangeFieldData/ApplyMappingMatrix.md) module to map the values from a field similar to the source field onto the destination field.

The source field for BuildMappingMatrix and [ApplyMappingMatrix](../ChangeFieldData/MapFieldDataFromSourceToDestination.md) must contain the same geometry and data locations, however they do not have to contain the same data values or value types. For instance, both a TetVolField of doubles with data at the nodes and a TetVolField of Vectors with data also at the nodes can be used as input fields for BuildMappingMatrix.

[ApplyMappingMatrix](../ChangeFieldData/MapFieldDataFromSourceToDestination.md) can be used instead of BuildMappingMatrix and ApplyMappingMatrix if the mapping is not to be reused, as it does the same interpolation without building the intermediate mapping matrix. This should be used if the source field changes much more often than the destination field.
