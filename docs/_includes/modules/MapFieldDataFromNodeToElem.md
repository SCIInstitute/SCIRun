---
title: MapFieldDataFromNodeToElem
category: moduledocs
module:
  category: ChangeFieldData
  package: SCIRun
tags: module
layout: null
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Compute the value of the elements based on the data from the adjoining elements. This module supports various operations to map node data to element data.

**Detailed Description**

Compute the value of the elements based on the data from the adjoining elements. This module supports various operations to map node data to element data.

Supported methods are:

  * AVERAGE - Compute the average of adjoining nodes
  * MIN - Compute the minimum value of adjoining nodes 
  * MAX - Compute the maximum value of adjoining nodes
  * SUM - Compute the sum of the adjoining nodes
  * INTERPOLATE - Compute a weighted average of the adjoining nodes

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
