---
title: CalculateMeshCenter
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

The module computes the center of a mesh based on a predefined method, chosen by the user.

**Detailed Description**

The module computes the center of a mesh and outputs it as a single node. The user can chose the method with which the center is determined from the following options-

**Average of Node Locations**
Node locations within the defined mesh are averaged. The output center will favor areas of high node density.

**Average of Element Locations**
Element locations within the defined mesh are averaged. The output center will favor areas of high element density.

**Volumetric Center (Default)**
Calculates the center of mass for the defined mesh, irrespective of node location, element location or Cartesian, bounding box.

**Bounding Box Center**
Calculates the center based on Cartesian bounding box dimensions. That is, a rectangular prisim region, that completely encompasses the mesh will be generated. The mesh center will be defined as the center of the rectangular prism mentioned above.

**Middle Index Node**
Defines the center as the central node index number. Consider a LatVol mesh of dimension 4 x 4 x 4. Such a mesh would have node index numbers 0 thru 63. The center would be defined as node number 32.

**Middle Index Element**
Defines the center as the central element index number. Consider the same 4x4x4 LatVol mesh from above. Such a mesh would have element index numbers 0 through 26. The center would be defined as element number 13.

**Note:** The above Middle Index examples would have the center defined at the volumes edge (Middle Node) or at the volumes center (Middle Element). Mesh centers are determined entirely by index numbering.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
