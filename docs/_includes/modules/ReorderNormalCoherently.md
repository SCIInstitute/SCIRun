---
title: ReorderNormalCoherently
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary


This module aligns the normals in one direction.


**Detailed Description**
This module aligns the normals of the meshes in one direction. 
It works for open surfaces and not for crossing surfaces. If there are any cross surfaces then they need to be isolated and then Reordering needs to be done.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
