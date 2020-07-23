---
title: BuildFEVolRHS
category: moduledocs
module:
  category: FiniteElements
  package: SCIRun
tags: module
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

### Summary

This module calculates the divergence of a vector field over the volume. It is designed to calculate the volume integral of the vector field (gradient of the potential in electrical simulations).

Then, it builds the volume portion of the RHS of FE calculations where the RHS of the function is GRAD dot F.

**Detailed Description**

The input is a FE mesh with field vectors distributed on the elements (constant basis). 

The output is the Grad dot F.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
