---
title: CalculateDistanceToField
category: moduledocs
module:
  category: ChangeFieldData
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module computes the minimum distance between an object (field or mesh) and the nodes in field or the center of the elements.

**Detailed Description**

This module computes the minimum distance between a node or an element (center of element) and a mesh. In case the object field is a point cloud the module computes the distance to the closest node, in case the object mesh is a curve it will find the closest distance to the curve, etc. The result is a field where the scalar values refer to the distances to the object field. The first input of this module is the field at which the distance field needs to be calculated, the second input port defines the field the distance to which needs to be calculated.

If the original field has the data located on the nodes of the mesh, these nodes are used to calculate the distance to the field object, if the data is located on the elements the distance to the center of the elements is calculated. If no data is stored in the field, the distance to the nodes is calculated.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
