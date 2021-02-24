---
title: WriteField
category: moduledocs
module:
  category: DataIO
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module saves persistent field objects received from upstream modules.

**Detailed Description**

Upon opening, the GUI defaults to the directory that the user sets for their SCIRUN_DATA environment variable. 

Otherwise, the GUI will default to the directory where the user's SCIRun executable resides. The user can enter the name of a file that the matrix saves to. The file should have a .fld extension. The default file name is MyField. The user may also choose between a Binary or ASCII file format.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}



