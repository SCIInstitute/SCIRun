---
title: GeneratePointSamplesFromFieldOrWidget
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

This module generates samples from any type of input field and outputs the samples as a **PointCloudField** field.

**Detailed Description**

GeneratePointSamplesFromFieldOrWidget generates samples from any type of input field and outputs the samples as a PointCloudField field. The samples can be generated randomly or user selected via a 3D widget. Use the **shift key/left mouse button** combination to manipulate the size (resize cylinders), orientation (spheres on the widgets are rotation points) and position of the widgets. Use the same key/mouse combination to move the slider on the rake widget. Use the **shift key/right mouse button** combination to bring up a dialog box to change widget scale.


The random sampling can be weighted by importance (determined by the data value of the sample) or left unweighted. The random sampling can also be limited to the selection of node points, rather than points from the interior of the fields elements, by clamping the samples to nodes.

The **uniform** distributions are a uniformly random function over the spacial extents of the field (i.e. the elements are weighted by their volume or area), whereas the **scattered** distributions are a uniformly random function over the elements only (i.e. all elements have the same weight). The **importance weighted** distributions multiply the existing weight of a sample by the interpolated data value associated with it, while the **not weighted** distributions leave the existing weight of a sample unchanged.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
