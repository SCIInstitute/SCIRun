---
title: ClipFieldByFunction
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

This module selects a subset of a field.

**Detailed Description**

The ClipFieldByFunction module clips out a subset of a field by preserving all of the elements in the input field for which the user specified expression evaluates to true at the specified test location. As a side effect of this algorithm is that any degenerate non-element items in the input field are discarded by the clip.

The type of the field is preserved in the clip, as well as the data values if possible. Some field types are structured and thus not clippable. This includes the LatVolField, ImageField, and Scanline field types. 

The expression should be a SCIRun parser expression. The module UI has a help button that will bring up documentation for the parser. For node centered data, select clipping location from One Node, Most Nodes or All Nodes. For cell centered data, select Element Center.

The ClipFieldByFunction module takes an **input** field, and optionally a function string and/or one or more additional matrices that may be used in the function.

The ClipFieldByFunction module has two **output** ports: the clipped field and a mapping matrix.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
