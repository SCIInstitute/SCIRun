---
title: BuildFESurfRHS
category: moduledocs
module:
  category: BIOPSE
  package: SCIRun
tags: module
layout: null
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Calculates the divergence of a vector field over a surface. It is designed to calculate the surface integral of the vector field (gradient of the potential in electrical simulations).

Builds the surface portion of the RHS of FE calculations where the RHS of the function is GRAD dot F.

**Detailed Description**

*Input*
 - A FE mesh with field vectors distributed on the elements (constant basis).

 *Output*
 - The Grad dot F 

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
