---
title: GetFieldData
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

This will get the data (scalar, vector, tensor) associated with the nodes or the elements of a Field and put them in a Matrix or Nrrd with the first entity corresponding to the first matrix entity.

**Detailed Description**

#### Input Port

A Field with data on the nodes or elements.

#### Output Ports

A Matrix, Nrrd, or complex Matrix with the corresponding data.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
