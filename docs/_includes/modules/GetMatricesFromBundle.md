---
title: GetMatricesFromBundle
category: moduledocs
module:
  category: Bundle
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module retrieves a **matrix** object from a bundle.

**Detailed Description**

This module retrieves a **matrix** object from a bundle by specifying the name under which the obect is stored in the bundle. The module has three output ports that each can be programmed to retrieve a specific **matrix** object.

There are two ways of specifying the name of the **matrix** object. Firstly one can enter the name of the object in the entry box on top of the menu, secondly one can execute the module, in which case a list of all objects of the **matrix** is generated. By selecting the one that one wants on the output port the obect can be retrieved from the bundle.

The first bundle output port generates a copy of the input bundle and can be used to attach a second module that retrieves data from the bundle.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
