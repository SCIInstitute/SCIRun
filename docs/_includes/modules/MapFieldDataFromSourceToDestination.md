---
title: MapFieldDataFromSourceToDestination
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

This moduled takes a field and finds data values for the destination geometry and outputs the resulting field.

**Detailed Description**

This modules takes 2 Fields as input:

1. **Source** contains geometry and data values.
2. **Destination** contains geometry only. 

This module will find the corresponding data value in the **Source** for each node in the **Destination** field. The data values can be stored at the nodes, cells, edges, or faces of the mesh at either the input or output ports. If the geometries of the input ports are different, the module will calculate data values for the Destination by interpolation.

This module returns 1 output Field:

1. **Remapped Destination** contains the **Destination** geometry with the **Source** input.

#### GUI Options

The user can select the option of choosing the closest element (**Constant** option) when a data value cannot be interpolated. Otherwise, **Linear** interpolation can be used. The **Maximum Distance** option indicates the maximum distance between points of interpolation and data values. The default for unassigned values can be numeric or NaN.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
