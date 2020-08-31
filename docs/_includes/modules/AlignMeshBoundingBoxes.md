---
title: AlignMeshBoundingBoxes
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Scales, translates, and deforms an *Input* field to a defined *Alignment* field based on the volumetric bounding boxes that encompass each of these fields.

**Detailed Description**

The module extracts the center and size dimensions of each field. The center of the input field is reassigned to center of the alignment field.
The size dimensions of the input field are likewise matched to those of the alignment field, proportionally reducing the Euclidean distance between nodes of the input field.
The result is a transformed input field that shares the same dimensions and position as the alignment field (without rotation).

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
