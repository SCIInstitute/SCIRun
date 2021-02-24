---
title: GetDomainBoundary
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

This module will extract the inner and outer boundaries from a mesh. 

**Detailed Description**

An **inner boundary** is defined as the boundary between different regions with a different value on the elements. 

The **outer boundary** is the boundary that surround the mesh. 

In the GUI one can selectively set which boundaries are extracted from the mesh. One can set the range of values on the segmented field for which one wants to extract the boundary separating the different compartments. Within this range of selected domains one can select only the inner boundaries to this selected domain or one can get the boundary surrounding the selected domains.

This module is intended to extract the boundaries in a segmented field. The module is templated and should work on any mesh type as long as the data is assigned to the elements.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
