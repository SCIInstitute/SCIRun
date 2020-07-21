---
title: GetFieldBoundary
category: moduledocs
module:
  category: NewField
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module extracts a boundary surface from a volume field.

**Detailed Description**

The GetFieldBoundary module extracts the bounding surface of the incoming field, making it into a new field that it outputs through the BoundaryField port. This module does not have a GUI. It has one input port, Input, and two output ports, **Boundary**, and **Mapping**. The GetFieldBoundary module builds an appropriate mesh from the incoming field. GetFieldBoundary will build a QuadSurfMesh, TriSurfMesh, or CurveMesh as appropriate for the boundary type of the input field. The Boundary output port will contain the new boundary mesh with no data interpolated onto it. If the data from the input field is desired on the output field, it is recommended to use the **Mapping** output, running the results through {% include moduleLink.md moduleName='ApplyMappingMatrix' %} in order to draw values from the input field.

Any interior face should be shared by two cells. The boundary surface is calculated as the module checks each face that is shared by two or more cells. The faces that exist in one cell only are boundary faces. This module can be used as a debugging tool becuase errors or holes in the mesh will show up as boundary faces.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
