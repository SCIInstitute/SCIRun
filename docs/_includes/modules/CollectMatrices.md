---
title: CollectMatrices
category: moduledocs
module:
  category: Math
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module appends/replaces a column or a row in/to a matrix while looping through a network.

**Detailed Description**

This module appends, replaces, or prepends a column or row of a matrix. This module does one of these operations each time the module is executed. Hence, the appending or replacing operation is in time.

This module has two input ports: 

  1. base matrix, which is read the first time the module is executed
  
  2. the matrix that needs to be appended is entered through the second input port

When the user wants to restart the network, he or she needs to clean this module as it would continue to collect matrices. To clear the matrices in the buffer of this module, hit the **Clear Output** button in the GUI, which will reset this module. The UI has options to allow the user to select:

Use the GUI to specify the operation which has to be performed each cycle of a network execution: the first column in the GUI specifies whether the module works on rows or columns, the second specifies whether one wants to add or replace the last column or row. The last column specifies whether a column/row is replaced at the start of the matrix (row 0 or column 0) or at the end of the matrix.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
