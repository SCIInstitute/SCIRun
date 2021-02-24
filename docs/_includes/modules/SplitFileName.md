---
title: SplitFileName
category: moduledocs
module:
  category: String
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module splits a filename in: pathname, filename (base), and extension.

**Detailed Description**

This module splits the complete filename into its three base components: pathname, filename, and extension. These three string are in the first, second, and third ports respectively. To get the full filename back, just merge the three resulting strings together. The forth port contains the filename and extension (i.e. removes the base path).

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
