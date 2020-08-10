---
title: GeneratePointSamplesFromField
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

This modules allows you to set seed points in a given field and interactively change their location.

**Detailed Description**

The **input** is a field containing a mesh.

The **Geometry** output connects directly to the ViewScene module and produces points that can be interactively moved by holding shift while dragging them. This will update the points output to the Field port.

The **Field** output the points as a point cloud field.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
