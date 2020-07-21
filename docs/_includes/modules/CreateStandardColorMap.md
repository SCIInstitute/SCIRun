---
title: CreateStandardColorMap
category: moduledocs
module:
  category: Visualization
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module generates fixed Colormaps for visualization tools.

**Detailed Description**

This module is used to create some "standard" non-editable colormaps in Dataflow/Uintah. Non-editable simply means that the colors cannot be interactively manipulated. The module does, allow for the the resolution of the colormaps to be changed. This class sets up the data structures for Colormaps and creates a module from which the user can choose from several popular colormaps. By clicking in the Color band the user manipulate the **transparency** of the color. This is useful, for example, when volume rendering, though many visualization tools ignore the transparency data.

Most of the important work for this module is performed in the CreateStandardColorMaps.tcl file. There you can easily add new colormaps by making the obvious changes to the buildColorMaps function and the UI function (where the make_labeled_radio buttons are created). The C++ code merely queries the tcl code and fills the Colormap.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
