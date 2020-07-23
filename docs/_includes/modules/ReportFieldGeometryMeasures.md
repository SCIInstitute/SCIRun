---
title: ReportFieldGeometryMeasures
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

Build a densematrix, where each row is a particular measure of the input Field (e.g. the x-values, or the element size).

**Detailed Description**

ReportFieldGeometryMeasures outputs a NxM matrix filled with values associated with the input mesh measures at selected mesh locations.

First the user selects the measues location in the top half of the UI. The number of measure locations equals the number of rows (N) in the output matrix.

Then the user selects what measures are sampled at each location in the bottom half of the UI. The number of measures selected equals the number of columns (M) in the output matrix.

#### Description of Measures:

  * **X position** - The X coordinate of the center of the measure location.

  * **Y position** - The Z coordinate of the center of the measure location.

  * **Z position** - The Z coordinate of the center of the measure location.

  * **Index** - The index used by the code to reference the measure location.

  * **Valence** - The # of similar measure locations that share a connection with the measure location. This varies by mesh and is not defned in all circumstances and thus equals 0 in these cases.

  * **Size** - The Euclidean Volume of the measure element. Ex: length for edges or 0.0 for points.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
