---
title: ReportMatrixInfo
category: moduledocs
module:
  category: Math
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module reports the attributes of matrices.

**Detailed Description**

ReportMatrixInfo is purely an informational Module. It performs no modification on input data. Upon execution it displays attributes about the input matrix in the UI.

Matrix Attributes displayed:

  * Name - The Matrix name. May be blank.

  * Generation - The Matrix internal object id.

  * Typename - The C++ typename of the input Matrix Type.

  * Rows - The number of rows in the input matrix.

  * Columns - The number of columns in the input matrix.

  * Elements - The size of the matrix. This is Rows x Columns for non sparse matrices. For sparse matrices this is the number of nonzero elements in the array.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
