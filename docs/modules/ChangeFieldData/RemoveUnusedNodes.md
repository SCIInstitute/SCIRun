# RemoveUnusedNodes

Removes unused nodes from an input mesh.

**Detailed Description**

Extracts element and node data from the input mesh. Passes through the element data and determines which nodes are being used to define the elements. Any nodes not used to define the elements is eliminated.

Can only be done on unstructured meshes with linear elements.
