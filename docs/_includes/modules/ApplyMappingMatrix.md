---
title: ApplyMappingMatrix
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

Apply a mapping matrix to project the data from one field onto the mesh of another field.

**Detailed Description**

Mapping data from a **source field** to a **destination field** can be done in a two stage process. First one builds a mapping matrix that describes which linear combination of data values of the source field needs to be taken to form a value in the destination field and secondly one multiplies the data vector of the source data with this matrix to obtain the destination data vector. This module accomplishes the second stage of this process. In order to build a mapping matrix use the module {% include moduleLink.md moduleName='BuildMappingMatrix' %} in the MiscField category. The reason for splitting this process is to improve performance of the mapping.

A second advantage of calculating the mapping matrix is that multiple mapping matrices can be built, which then can be multiplied to create the mapping matrix that spans a series of mesh operations.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
