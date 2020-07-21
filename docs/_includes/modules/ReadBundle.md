---
title: ReadBundle
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

This module reads a **bundle** from file.

**Detailed Description**

This module reads a **bundle** from file. A bundle file has a .bdl extension and can be written with the WriteBundle module. In a .bdl file every component is stored. Hence it can be used to group a lot of different SCIRun objects together and store it in one file.

This module has one input port through which the user can define the name of the file that needs to be read. If this port is connected to a string dataflow object, this name is used instead the one entered in the GUI.

This module has two output ports: the first port contains the **bundle** that was read and the second port contains a copy of the **filename**. Hence, if one wants to annotate the results, this filename can be displayed using the ShowString module.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
