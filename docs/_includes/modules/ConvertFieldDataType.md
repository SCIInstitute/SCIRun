---
title: ConvertFieldDataType
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

ConvertFieldDataType is used to change the type of data associated with the field elements.

**Detailed Description**

ConvertFieldDataType allows the user to change the datatype at each field data_at location. This module also attempts to convert the data in the field to the new type, if possible. If no conversion is possible, the output field will contain default zero values. Note that conversion from a large datatype to a smaller datatype usually results in a loss of precision.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
