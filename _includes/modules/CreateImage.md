---
title: CreateImage
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


This module makes an **ImageField** that fits the source field.


**Detailed Description**

Makes an ImageField that fits the source field. The value type of the ImageField is same as that of the input field. If there is no input field specified, then it creates a unit volume of doubles. The size parameters refer to the number of nodes in the volume, not the number of faces. Thus a 2x2x2 node lattice will only contain one facet.

The **Pad Percentage** parameter is an optional parameter that describes how much larger than the input field the resulting lattice volume should be. For example, a value of 100 would make the image field be three times as far across and contain nine times the area of a image with the default 0 padding. A value of 50 would cause the image to be twice as far across (50% bigger on each side).

No data is generated. In order to map the data mapping modules from SCIRun{% include moduleLink.md moduleName='MapFieldDataOntoElements' %} or {% include moduleLink.md moduleName='MapFieldDataOntoNodes' %} can be used if those values are needed.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
