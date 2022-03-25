---
title: ResizeMatrix
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

This module lets the user resize their matrix to the desired size.

**Detailed Description**

This module lets the user resize their matrix to the desired size. In the GUI, one can choose the size of the matrix (by defining the number of rows and columns). If the number of elements in the original matrix is less than the (no. of column)X(no. of rows) then the new matrix will be padded with zeros. If the number of elements in the original matrix are more than the (no. of column)X(no. of rows) then the matrix will be truncated. Users can also choose between the row or columns major ordering of the resizing.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}

