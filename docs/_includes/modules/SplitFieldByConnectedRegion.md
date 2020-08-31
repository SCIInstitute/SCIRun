---
title: SplitFieldByConnectedRegion
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

This module splits a domain into separate fields as defined by the input field's connectivity.

**Detailed Description**

This module divides a single input field into as many as 8 output regions. Separate regions may also be collected and bundled. Connected regions are not defined by label masks. They are defined, rather, by node and/or element connectivity. For example, a continuous, tetrahedral mesh may use label masks to define domains, but this module will produce only one output field given that all tets within the mesh are connected. 

A common use for this module, then, is to separate distinct field surfaces, to split away small mesh islands that are not connected to the main mesh, or to break up a fractionated mesh into its individual regions. 

Domains can be sorted in two ways: **Sort Domains by Size** or **Sort Ascending**. Domain size refers to the overall surface area (if domains are surfaces) or volume (if domains are 3-dimensional). The ascending sort refers to the value assigned to each domain in the original field.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
