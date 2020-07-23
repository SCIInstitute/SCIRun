---
title: SetFieldDataToConstantValue
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

This module sets field data to a given scalar value on a new output field based on the input field geometry.

**Detailed Description**

This module sets field data to a scalar value provided by the user in the GUI on a new output field created with the same geometry as the input field. The field **basis** type can be changed in the output field, or left the same as the input field (default). The scalar value can be set to any type supported by SCIRun (char, short, unsigned short, unsigned int, int, float, double), or left the same as the input field (default).

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
