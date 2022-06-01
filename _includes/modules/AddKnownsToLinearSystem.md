---
title: AddKnownsToLinearSystem
category: moduledocs
module:
  category: Math
  package: SCIRun
tags: module
layout: null
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module deals with solving a linear system ```A*u=b``` when some values of the vector u is already known. The module will modify the linear system according to the known values.

**Detailed Description**

This module takes 3 inputs:

1. **LHS Matrix** is an NxN Matrix (must be a SparseRowMatrix).
2. **RHS Vector** is the right hand side vector, an Nx1 Matrix.
3. **X Vector** is an Nx1 Matrix specifying the known variables in the linear system LHS. If the k*th* variable is known, ```x(k)``` is its value, otherwise ```x(k)``` should be set NaN.

This module returns 2 outputs:

1. **OutPutLHSMatrix** is an NxN Matrix.
2. **OutPutRHSVector** is an Nx1 Matrix.

The solution:

```
inv(A2)*b2
```

has the same value as x at the non-NaN positions.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
