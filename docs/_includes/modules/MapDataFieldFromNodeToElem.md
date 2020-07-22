---
title: MapDataFieldFromNodeToElem
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

This module computes the value of the elements based on the data from the adjoining elements. This module supports various operations to map node data to element data.

**Detailed Description**

Supported methods are: 

  * AVERAGE - Compute the average of adjoining nodes 
  * MIN - Compute the minimum value of adjoining nodes
  * MAX - Compute the maximum value of adjoining nodes
  * SUM - Compute the sum of the adjoining nodes
  * INTERPOLATE - Compute a weighted average of the adjoining nodes

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
