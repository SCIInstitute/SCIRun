---
title: FairMesh
category: moduledocs
module:
  category: NewField
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module smooths surface meshes without shrinking them.

**Detailed Description**

Based on the surface smoothing algorithm published by Taubin in 1995 (A Signal Processing Appoach to Fair Surface Design).

User Interface:

  * Weighting methods: Equal and curvature normals (default: equal)
  * Iterations: Number of times the surface is put through the smoothing algorithm (default: 50)
  * Spatial cut off frequency: Similar to low pass filter setting (default: 0.1)
  * Relaxation Parameter: negative scale factor -- the shrinking term (default: 0.6307)
  * Note: an unshrinking term is produced by: Spatial cutoff = 1/Relaxation parameter + 1/Dilation parameter

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
