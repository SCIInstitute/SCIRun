---
title: CreateLatVol
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


This module makes a **LatVolField** that fits the source field.

**Detailed Description**

Make a LatVolField that fits the source field. The value type of the LatVolField is the same as that of the input field. If there is no input field specified, then create a unit volume of doubles. The size parameters refer to the number of nodes in the volume, not the number of cells. Thus a 2x2x2 node lattice will only contain one cell.

The **Pad Percentage** parameter is an optional parameter that describes how much larger than the input field the resulting lattice volume should be. For example, a value of 100 would make the lattice volume be three times as far across and contain twenty seven times the volume of a lattice with the default 0 padding. A value of 50 would cause the lattice to be twice as far across (50% bigger on each side).

No interpolation is done onto the new field. It is recommended that the {% include moduleLink.md moduleName='MapFieldDataOntoElements' %} or {% include moduleLink.md moduleName='MapFieldDataOntoNodes' %} module be used if those values are needed.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
