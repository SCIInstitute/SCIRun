---
title: CalculateCurrentDensity
category: moduledocs
module:
  category: Forward
  package: SCIRun
tags: module
layout: null
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Compute the current density vector field

**Detailed Description**

CalculateCurrentDensity calculates the current density vector field.

Required inputs are the electric field and mesh with conductivities.

Technical note: The current density vector field J is the product of sigma and -del V. The minus sign is added in CalculateCurrentDensity, so the electric field input should be positive (which is the unmodified output of the {% include moduleLink.md moduleName='ChangeFieldData' %}::{% include moduleLink.md moduleName='CalculateGradients' %} module). 

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
