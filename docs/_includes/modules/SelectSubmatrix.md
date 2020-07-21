---
title: SelectSubmatrix
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

This module chooses a submatrix based on user input. The matrix may be chosen by column and/or row ranges through the user interface or by array or matrix inputs defining desired row or column indices.

**Detailed Description**

There are two methods to chose a submatrix:

**User Interface method:** The user may select a range of rows/columns by opening the user interface window, selecting the check box associated with rows or columns and providing the desired range. Default values are -1 and does not clip the matrix.

**Input Matrices method:** Two additional input ports allow the user to define arrays or matrices of desired matrix indices. If the row or column matrices are nx1, the module will output a new matrix with rows/columns defined by the values in of the array. If matrices are NxM it will output a new matrix with rows/columns defined by the matrix values in a left-to-right top-to-bottom manner. If the input matrix has non-integer values, the number will be truncated (not rounded) and only the integer value will be used.

Examples:

  * *Removing columns:*
Columns can be removed from the front or the back of the matrix using the user interface. Alternatively, a matrix can be fed into the 'Matrix ColumnIndices' input port and will be defined by the rules in the above paragraph. Rules apply to the row input port.


  * *Re-ordering rows:*
Rows can be re-ordered by supplying the 'Matrix RowIndices' with a matrix that defines the order in which the user wants the rows to be ordered. Input port ordering is defined in the paragraph above. Rules apply to the column input port.


  * *Cherry-picking matrix values:*
By supplying matrices to both 'Matrix ColumnIndices' and 'Matrix RowIndices' values for the output matrix will be defined by the associated row and column values.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
