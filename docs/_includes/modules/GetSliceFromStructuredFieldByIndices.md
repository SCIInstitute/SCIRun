---
title: GetSliceFromStructuredFieldByIndices
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

This module reduces the dimension of a topologically regular field by 1 dimension.

**Detailed Description**

The GetSliceFromStructuredFieldByIndices **input** port is a field that has regular topology. The field will be reduced by 1 dimension and piped to the output port.

The user must supply two input values either via the module GUI, or from an optional input matrix: the axis the field is reduced along, and location along the selected axis. If supplying an matrix of clipping locations, the matrix must either be 1x1 or 3x3 and have data entries that follow this pattern:

  * If 3x3 matrix, row index is the axis: row 0 = axis i, row 1 = axis j, row 2 = axis k (only select one at a time)

  * Column 0 is the selected axis to slice

  * Column 1 is the slice index (location along the selected axis where the field will be sliced)

  * Column 2 is the data dimensions
  
A 3x3 matrix containing selected axis, slice index and data dimensions following the above pattern is also output.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
