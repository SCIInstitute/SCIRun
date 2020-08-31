---
title: ScaleFieldMeshAndData
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Applies a scale factor to the node locations of the data and/or the data itself. The scale factor can be multiplied to nodes by shifting the centroid to zero and then shifting it back. Or the scale factor can be applied without bringing the centroid to zero. The latter will scale the mesh around zero, and the former will scale the mesh about it's centroid.

**Detailed Description**

Input requires a mesh in a field format with or without data assigned to the nodes or elements. The second and third inputs are optional matrix fields were you can input the scale factor for the mesh and the scale factor for the data.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
