---
title: SplitFieldByDomain
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

This module splits a domain with predefined domains (i.e. label masks) into separate fields.

**Detailed Description**

This module accepts a single input field with predefined domains and splits each domain into separate fields. New fields are output as individual fields, or as a bundle of distinct fields. Up to 8 distinct possible output fields are available per module. If additional splits are needed, the module can be repeated as often as necessary on the final output field port. Domains can be sorted in two ways: **Sort Domains by Size** or **Sort Ascending**. 

Domain size refers to the overall surface area (if domains are surfaces) or volume (if domains are 3-dimensional). 

The ascending sort refers to the label mask that defines each domain in the original field. 

The resulting output will be ordered relative to the values used to define the input field's domains.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
