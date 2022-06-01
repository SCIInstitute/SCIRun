---
title: BuildMatrixOfSurfaceNormals
category: moduledocs
module:
  category: MiscField
  package: SCIRun
tags: module
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

### Summary

This module calculates area weighted normal vectors per node.

**Detailed Description**

For each node in the input surface mesh, find the attached faces, and average the face normal weighted by area for that node. Output a Nx3 DenseMatrix where N is the number of nodes in the input surface mesh. This matrix can be passed to {% include moduleLink.md moduleName='SwapFieldDataWithMatrixEntries' %} to map this data back to a vector field with the same mesh.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
