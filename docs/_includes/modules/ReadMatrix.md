---
title: ReadMatrix
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

This module reads a persistent matrix from a file and outputs that matrix to another module.

**Detailed Description**

Upon opening, the GUI defaults to the directory that the user sets for their SCIRUN_DATA environment variable. Otherwise, the GUI will default to the directory where the user's SCIRun executable resides. 

The current directory defaults to only show files with an .mat extension which helps the user determine the difference between matrix files and other files. However, the ReadMatrix can read-in a file with any extension so long as the data has the correct format. 

If the user attempts to read-in a file that is other than a SCIRun supported matrix, or uses an incorrect file format, an error message appears.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
