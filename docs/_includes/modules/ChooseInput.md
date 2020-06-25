---
title: ChooseInput
category: moduledocs
module:
  category: FlowControl
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Choose one of any number of inputs (of any type) to send to the ouput.  

**Detailed Description**

The ChooseInput module allows users to send any one of several inputs as an output to the module.  This module can be used to quickly switch between data sent to other modules.  Input Data can be of any type (Field, Matrix, String, etc), and the output will be the type (and the same data) of the chosen input.  Choose which input to send by setting the input port number in the module UI.  There are no constraints on type or size of the input data, therefore the user should consider the output destination ensure that all the input data to the ChooseInput module is compatible with the destination port.   

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
