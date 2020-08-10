---
title: MapFieldDataOntoNodes
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

This module maps data from one mesh or point cloud to another mesh or point cloud. The output mesh will have the data located at the nodes.

**Detailed Description**

This modules can interpolate a scalar field, or calculate and interpolate the gradient, gradientnorm, or flux.

The first input is the **source** field. The second input allows you to specify the **weights** of the interpolation. 

The output is the **target** field containing the interpolated data on the elements.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
