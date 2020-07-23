---
title: ConvertMeshToPointCloud
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

Convert input mesh to a PointCloudMesh.

**Detailed Description**

This converts a mesh to a PointCloudMesh (will not work on non-linear basis). If the input field is a PointCloudMesh, the field will be passed through to the output port. Data values and locations are preserved in the transformation.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
