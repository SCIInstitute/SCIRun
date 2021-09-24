---
title: ConvertNrrdToMatrix
category: moduledocs
module:
  category: Converters
  package: Teem
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module takes 3 optional Nrrd input ports and puts the data into a matrix.

**Detailed Description**

Nrrd is both a library and a file format. The full documentation can be found at http://teem.sourceforge.net/nrrd/index.html

This module may convert the Nrrd data to a ColumnMatrix, DenseMatrix, or SparseMatrix depending on which ports are connected. The 3 input ports are Data, Rows, and Columns, respectively.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
