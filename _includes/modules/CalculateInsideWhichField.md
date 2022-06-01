---
title: CalculateInsideWhichField
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

This module detects whether a node or a cell is inside any of the object fields. The output field will be indexed according to the object it is in, starting with index 1. Index 0 is reserved for the part of the field not in any of the object fields.

**Detailed Description**

This module detects whether a node or a cell is inside any of the object fields. The module uses the location of the node or the center of the element to test whether a node or element is inside any of the object fields. The output field will be indexed according to the object it is in, starting with index 1. Index 0 is reserved for the part of the field not in any of the object fields. The module has two options in the GUI, the first one determines whether the detection of the position has to be done for nodes or for elements and the second option allows the user to change the type of field that is generated, the output data will be cased to the selected output type. Also there are options to choose the sampling scheme and the sample points that need to be inside the field.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
