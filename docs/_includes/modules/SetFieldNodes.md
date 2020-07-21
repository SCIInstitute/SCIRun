---
title: SetFieldNodes
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module
authors: Jeroen Stinstra
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Changes the location of the nodes in a mesh.

**Detailed Description**

SetFieldNodes replaces the nodes in a mesh with nodes provided in a matrix. This handles most mesh types supported by SCIRun, but the simplest application is with tri or quad surface meshes. This module very simply replaces the old nodes with the new ones, without regard for quality of elements or other considerations, so care must be taken to ensure the nodes are in the same order.

The inputs of SetFieldNodes are the original mesh with nodes and connections, and the matrix of new nodes. The matrix must be Nx3, where N is the number of nodes in the original mesh, so that there is an x,y, and z coordinate for each node. The output is the mesh with the new node locations.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
