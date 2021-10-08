# ShowMeshBoundingBox

The ShowMeshBoundingBox Module renders a semented red-green-blue 3D grid around an arbitrary field  

**Detailed Description**

ShowMeshBoundingBox queries the generic mesh interface to get the BBox of the Field, and then renders a "cage" of that bounding box. The user specifies nx/ny/nz (number of "bars" in each direction) via text entry in UI. The cage is rendered with red/green/blue lines corresponding to x/y/z.
