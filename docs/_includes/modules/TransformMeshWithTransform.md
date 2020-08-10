---
title: TransformMeshWithTransform
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module
author: Michael Callahan
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Non-interactive geometric transform of a field.

**Detailed Description**

This module is used to non-interactively transform the geometry of a field. The transform is passed in to the **Transform Matrix** port as a 4x4 matrix. There is no GUI for this module. The transform is generally computed by a different module, such as {% include moduleLink.md moduleName='AlignMeshBoundingBoxes' %}. For interactive geometry transforms, use the {% include moduleLink.md moduleName='CreateGeometricTransform' %} module.

**Note that for fields containing vectors and tensors, the direction of the data will be altered by the transform.**

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
