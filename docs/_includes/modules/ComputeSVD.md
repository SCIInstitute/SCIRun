---
title: ComputeSVD
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

This module computes the singular value decomposition (SVD) of a matrix.

**Detailed Description**

This module takes a matrix as input, converts it to a dense matrix, computes its SVD, and outputs its three SVD matrices. The three matrices are the left singular vectors (stored in the **columns** of 'LeftSingularMat'), singular values (in decreasing order, stored in the column matrix "SingularVals"), and the right singular vectors (stored in the **rows** of "RightSingularMat"). The number of elements in "SingularVals" is equal to the smallest dimension of the input matrix.

This module requires a version of SCIRun built with LAPACK support.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
