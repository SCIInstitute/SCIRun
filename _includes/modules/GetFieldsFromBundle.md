---
title: GetFieldsFromBundle
category: moduledocs
module:
  category: Bundle
  package: SCIRun
tags: module
---
### Summary

This module retrieves a **field** object from a bundle.

**Detailed Description**

This module retrieves a **field** object from a bundle by specifying the name under which the object is stored in the bundle. The module has three output ports that each can be programmed to retrieve a specific field object.

There are two ways of specifying the name of the **field** object. Firstly one can enter the name of the object in the entry box on top of the menu, secondly one can execute the module, in which case a list of all objects of the **field** is generated. By selecting the one that one wants on the output port the object can be retrieved from the bundle.

The first bundle output port generates a copy of the input bundle and can be used to attach a second module that retrieves data from the bundle.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
