---
title: SolveMinNormLeastSqSystem
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

This module computes the minimal norm, least squared solution to a Nx3 linear system.

**Detailed Description**

Given four input ColumnMatrices (v0,v1,v2,b), it finds the three coefficients (w0,w1,w2) that minimize:

```
       | (w0v0 + w1v1 + w2v2) - b |
```

If more than one minimum exists (the system is under-determined), the coefficients such that (w0,w1,w2) has a minimum norm is selected.

The outputs are a vector (w0,w1,w2) as a row-matrix and the ColumnMatrix (called x), which is:

```
       [ w0v0 + w1v1 + w2v2 ]
```

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
