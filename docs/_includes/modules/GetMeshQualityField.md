---
title: GetMeshQualityField
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

Calculates mesh quality based on several generally accepted algorithms.  Default: Scaled Jacobian

**Detailed Description**

Mesh quality reporting options:
Jacobian/Scaled Jacobian -- Jacobian and Scaled Jacobian are based on accepted mesh quality metrics as calculated by the Verdict software library.  In brief, for a single node, the Jacobian matrix is defined as:

```
                        | x_1 - x_0     x_2 - x_0     x_3 - x_0 |
 A_0 =                  | y_1 - y_0     y_2 - y_0     y_3 - y_0 |
                        | z_1 - z_0     z_2 - z_0     z_3 - z_0 |
```

The minimal determinant of these matrices for each node of an element is known as the 'Jacobian' of the element. (Callahan et al. 2009)

Volume -- Returns the volume of each element.

Scaled Inscribed/Circumscribed Ratio -- Mesh quality is determined by the ratio between the radii of the largest possible inscribed sphere compared to the smallest possible circumscribed sphere of each tetrahedral element.  The radius is normalized against he ratio of an equilateral tetrahedron 1:3.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
