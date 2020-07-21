---
title: MapFieldDataOntoElements
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

This module maps data from one mesh to another mesh. The output mesh will have the data located on the elements.

**Detailed Description**

This modules can interpolate a scalar field, or calculate and interpolate the gradient, gradientnorm, or flux.

The first input is the **source** field. The second input allows you to specify the **weights** of the interpolation. 

The output is the **target** field containing the interpolated data on the elements.

The GUI options allow you to change if the data is interpolated or taken from the closest element. You can also change how the sampling works within each element.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
