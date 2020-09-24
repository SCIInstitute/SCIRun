---
title: RegisterWithCorrespondences
category: moduledocs
module:
  category: ChangeFieldData
  package: SCIRun
tags: module
layout: null
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Removes unused nodes from an input mesh.

**Detailed Description**

Extracts element and node data from the input mesh. Passes through the element data and determines which nodes are being used to define the elements. Any nodes not used to define the elements is eliminated.

Can only be done on unstructured meshes with linear elements.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
