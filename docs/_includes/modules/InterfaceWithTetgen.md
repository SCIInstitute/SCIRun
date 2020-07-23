---
title: InterfaceWithTetgen
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

This module is a port to open and run Tetgen, a delauney tetrahedralization software package, within SCIRun.

**Detailed Description**

InterfaceWithTetgen is a module that will make a tetrahedral mesh from a trisurf mesh or a point cloud. This is the easiest way to turn a surface mesh into a volume in SCIRun. Using delauney tetrahedralization, tetgen will find the tetrahedral mesh that will connect the input points with the highest quality elements, i.e., the element face area are as equal as possible. For more information about tetgen an its capabilities, please refer to the [Tetgen website](http://wias-berlin.de/software/tetgen/).

In order for tetgen to run effectively, the flags must be properly set. If using a surface input, the mesh must be a valid trisurf mesh that is completely inclose and without overlapping/crossing elements. If using only a point cloud input, the first option (Tetrahedralize a piecewise linear complex (PLC)) must be disabled. In the case of a point cloud only input, the output will be a convex hull of the points. If this module is taking too long (several minutes) and you are not sure why, try getting rid of any quality or size constraints (should finish quickly, as fast as a few seconds depending on the mesh) and then gradually reintroducing them. The few the options enabled, the faster and more likely to solve the tetrahedralization.

There are four inputs:

  1. **Main (Required):** The main field to be processed by tetgen. The outer surface or Tetrahedral volume field to refine.

  2. **Points (Optional):** If there is a PointCloud attached to this input, the point in it will be included in the output tetvol. Data on this field is meaningless.

  3. **Region Attribs (Optional):** If a PointCloud is attached the points will be considered points inside the different regions. The data will be used as the volume constraint for that region. For the volume constraint to be respected you must pass the 'a' command line switch.

  4. **Regions (optional):** This is a dynamic input, each input should be a surface field, that defines a region inside the Main Input.


The module outputs a **tetrahedral mesh**.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
