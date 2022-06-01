---
title: ProjectPointsOntoMesh
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module
author: Jeroen Stinstra
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Project a point cloud onto a mesh

**Detailed Description**

ProjectPointsOntoMesh transforms individual points of a point cloud onto the surface of a mesh. This is a simple projection that finds the locations on the mesh (surface or point depending on the settings) that are closest to each point in the point cloud and uses those locations as the new coordinates for the respective points in the point cloud.

There are two options for projection; projecting points onto the elements or nodes of the mesh. Projecting onto the elements of the mesh will allow the points to be anywhere on the mesh. Projecting onto the nodes will ensure the new points will a subset of the mesh points.

There are two inputs to the module. The first input (Field) is the point cloud. The second input (Object) is the mesh onto which the points will be projected.

The output of the module is the projected point cloud.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
