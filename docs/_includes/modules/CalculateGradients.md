---
title: CalculateGradients
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

This module computes the derivative of a scalar field and output it as a vector field.

**Detailed Description**

The CalculateGradients module computes the derivative of a scalar field and converts it to a vector field. The gradient is the derivative of three dimensions; the first component in the X direction, the second component in the Y direction, and the third component in the Z direction grid, computing the general direction of "flow" at a specific point on the geometry.

Not all field types are supported by the CalculateGradients module. At this time only derivatives of TetVolMesh and LatVolMesh are supported. The CalculateGradients module has no GUI. If CalculateGradients recieves as input a Field type it does not support, an error message appears in the Error frame of the SCIRun environment.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
