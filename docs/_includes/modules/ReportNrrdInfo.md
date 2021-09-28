---
title: ReportNrrdInfo
category: moduledocs
module:
  category: Misc
  package: Teem
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module is used to view the attributes of Nrrd data.

**Detailed Description**

Nrrd is both a library and a file format. The full documentation can be found at http://teem.sourceforge.net/nrrd/index.html

ReportNrrdInfo is purely an informational Module. It performs no modification on input data. Upon execution it displays attributes about the input Nrrd in the UI.

#### Field Attributes displayed:

  * **Name** - The Nrrd name.

  * **Type** - The data format.

  * **Dimensions** - The dimensionality of the data.

  * **Origin** - The position of the center of the data.

  * **Spacing** - The amount of space between each point along a given axis.

  * **Size** - The number of samples along a given axis.

  * **Min** - The lower bound of the size in model space.

  * **Max** - The upper bound of the size in model space.
  
  There are more attributes that are not reported from this module. The other attributes are listed at http://teem.sourceforge.net/nrrd/format.html.
  
{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
