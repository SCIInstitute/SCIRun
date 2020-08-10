---
title: FlipSurfaceNormals
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module
author: Darrell Swenson
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This modules changes the normal of the face of an element.

**Detailed Description**

Changes the normal of the face of an element on a surface mesh by reordering how the nodes are ordered in the face definition. It takes a surface of triangles or quads in the field format as an input and outputs the same surface with the normals going in the other direction.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
