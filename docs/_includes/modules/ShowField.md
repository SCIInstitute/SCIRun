---
title: ShowField
category: moduledocs
module:
  category: Visualization
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module visualizes the geometry that makes up a **Mesh** inside a Field. When possible and selected, the field takes its color from the data values that permeate the field.

**Detailed Description**

The field in the first input port holds the mesh that is to be visualized. By default is will be displayed using the the default colow, which is editable from the UI. If there is a color map attached to the second input port, and there is valid data in field, then the data can be used as an index into the color map, and the mesh is rendered with appropriate colors. In addition, if there is valid data is field the data itself can be converted into a color. Scalar data creates a gray scale mapping, vector data (normalized) creates RGB colors, and the principle Eigen Vector (normalized) of tensor data also creates RBG colors.

**Nodes** can be rendered as points (default) or as spheres. **Edges** can be rendered as lines (default) or cylinders. If the nodes and edges are rendered as spheres and cylinders respectively they may be sized. **Faces** and **Text** can be rendered or not.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
