---
title: EditMeshBoundingBox
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module
author: Mckay Davis
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

EditMeshBoundingBox is used to transform the field geometry.

**Detailed Description**

EditMeshBoundingBox can trasform the field geometry via the UI or a widget attached to a ViewScene window.

The output field is the transformed input field.

The output matrix is the resultant transformation matrix used to transform the field. It can be used to transform other fields in SCIRun.

The UI can change the center and the size of the output field. Check the box next to the output attribute you wish to modify. The 'Copy Input to Output' button will reset the output values to the input values.

To interactively transform the input field you must attach the Widget Port (the middle output port) to the viewer window.

To move the widget: Shift+LeftClick on the edges of the widget frame and move the transformation box around.

To scale the widget: Shift+LeftClick on the spheres located on each face of the widget and drag. This will scale the output field in a direction normal to the face the sphere is located on.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
