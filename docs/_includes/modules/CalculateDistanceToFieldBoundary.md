---
title: CalculateDistanceToFieldBoundary
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

This module computes the minimum distance between the boundary of the field and the nodes in field or the center of the elements.

**Detailed Description**

This module computes the minimum distance between the boundary of the field and the nodes in field or the center of the elements. The first input of this module is the field on which we want to calculate the distance, the second input port is the field/mesh the distance to which we want to compute. If the input field has data contained on the nodes, the distance to the nodes of the mesh will be calculated, if the data is located at the elements, the distance to the center of the element will be calculated. If the input field contains no data, the distance to its nodes will be calculated.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
