---
title: ClipFieldByMesh
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

This module clips a mesh to another mesh.

**Detailed Description**

Clip the first input to the second mesh. Both meshes need to be volumetric meshes such as tetrahedral rather than surface meshes.

The output is the clipped mesh and a mapping matrix that allows you to apply data to the clipped mesh from the original using the module {% include moduleLink.md moduleName='ApplyMappingMatrix' %}.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
