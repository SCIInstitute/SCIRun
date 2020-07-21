---
title: SwapFieldDataWithMatrixEntries
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

This module adds and removes data from a field.

**Detailed Description**

This module performs two simultaneous operations.

First the **Input Field** is split into its mesh part and its data values. The data values are packaged up and passed to the **Output Matrix** port.

Second the input field geometry is joined with the data from the **Input Matrix**, and the result is passed out on the **Output Field** port. If the input matrix is not present, then the second operation is not performed and the Output Field is the same as the Input Field.

The format of the **Output Matrix** will be a column matrix if the input field was of scalar type. It will be an Nx3 matrix if the input field contained vectors. If the field contained tensors it will be Nx9 matrix, where the tensor is flattened out in left to right, top to bottom order. Column 0 contains (0, 0), column 1 contains (0, 1), column 2 contains (0,2), column 3 contains (1, 0), etc.

The **Input Matrix** should have the same number of values as the field where the values are to be stored. The type of the Intput Field is preserved in the Output field as well. So for instance if the input field is a vector field, the Output Field will also be a vector field, and the input matrix should be an Nx3 matrix where N is equal to the number of elements to be filled in.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
