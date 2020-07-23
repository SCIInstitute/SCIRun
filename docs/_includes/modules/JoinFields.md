---
title: JoinFields
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

This module glues any number of input fields into one output field.

**Detailed Description**

JoinFields takes in an arbitrary number of input fields and gathers them all up into one output field. If the input fields all have the same editable mesh type then the output field will also be of the same type. If the meshes are not editable (LatVol, Image and Scanline) the mesh type is not perserved because they cannot be arbitrarily glued together while maintaining their mesh type. Instead, appropriate equivalent mesh types will be produced (HexVol, QuadSurf). 

Options available through the UI include:

  1. Force PointCloudField as output

  2. Merge duplicate nodes (default)

  3. Merge duplicate elements

  4. Only merge nodes with same value

  5. Merge mesh only, do not assign values

The user may also set a tolerance defining the distance between nodes and/or elements to be merged.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
