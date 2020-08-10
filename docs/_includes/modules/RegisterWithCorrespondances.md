---
title: RegisterWithCorrespondances
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

This module allows you to morph using a thin plate spline algorithm one point set or mesh to another point set or mesh. It also has the option for a rigid transformation. Both options require correspondence points from both point sets. This module also requires SCIRun to be compiled with LAPACK or Blas.

**Detailed Description**

  * InputField: This will read the node locations from an input mesh that is to be transformed.

  * Correspondences1: This reads the node locations from a field of the correspondence points in the new coordinate system

  * Correspondences2: This reads the node locations from a field in the same coordinate system as the InputField

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
