# GenerateNodeNormals

Make a new vector field that points to the input point

**Detailed Description**

This creates a new vector field containing the same geometry and data location as the input field. It works in two modes. If the input PointCloudField contains only one point, all of the vectors in the output field will be attracted to that input point. If the input PointCloudField contains two or more points, the first two points will be used as a line and all the vectors in the output field will be attracted to that line. If the input field contains scalar values, the vectors in the output field will be scaled by those values. Otherwise they will all be normalized.
