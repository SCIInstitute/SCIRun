---
title: ReportFieldInfo
category: moduledocs
module:
  category: MiscField
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module is used to view the attributes of fields.

**Detailed Description**

ReportFieldInfo is purely an informational Module. It performs no modification on input data. Upon execution it displays attributes about the input field in the UI.

#### Field Attributes displayed:

  * **Name** - The Field name. May be blank.

  * **Generation** - The Field internal object id.

  * **Typename** - The C++ typename of the input Field Type.

  * **Center** - The X,Y,Z coordinates of the average center of the nodes in the Field.

  * **Size** - The X,Y,Z coordinates of the grid-aligned bounding box that encloses the Field.

  * **Data min,comax** - Only valid for scalar type input fields. Prints the respective min and max scalar values of the data associated with the input field.

  * **Nodes** - The number of X,Y,Z points in the input field.

  * **Elements** - The number of highest dimension elements in the input field. Ex. For a PointCloudField this would equal the number of nodes. For a TetVolField this would equal the number of tetrahedral cells in the input field.

  * **Data at** - The location of the data values associated with the input field.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
