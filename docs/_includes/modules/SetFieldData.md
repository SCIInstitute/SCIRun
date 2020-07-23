---
title: SetFieldData
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

This module allows you to set the scalar, vector, or tensor entries of an array or single column matrix to the nodes or elements of a mesh. The module checks the size of the array and compares it to the number of elements and nodes in order to determine where to put the data. The first entry in the array is applied to the first location on the mesh and this continues sequentially.

**Detailed Description**


The first input takes a field and it expects a mesh or a point set.

The second input takes an array of data or matrix.

The third input takes NRRD data: this can be used instead in place of the matrix.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
