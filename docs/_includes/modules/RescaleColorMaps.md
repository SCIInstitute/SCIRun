---
title: RescaleColorMaps
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

This module allows the user to manually or automatically set the range of scalar values that map to the ColorMap. RescaleColorMap is an example of a module that has dynamic ports, because each time the user connects a Field module to the RescaleColorMap Field input port, another Field input port appears.

**Detailed Description**

In the GUI, the **Auto Scale** check box uses the minimum and maximum values found in the ScalarField and maps the ColorMap to that range. The **Fixed Scale** check box allows the user to manually select for the range of scalar values to which the colors map by adjusting the minimum and maximum values in the appropriate text fields. After changing the range to which the colors map, a new ColorMap passes downstream to the connecting module.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
