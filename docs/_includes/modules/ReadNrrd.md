---
title: ReadNrrd
category: moduledocs
module:
  category: DataIO
  package: Teem
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module allows the user to load Nrrd files and then sets that Nrrd data in the module's output port.

**Detailed Description**

Nrrd is both a library and a file format. The full documentation can be found at http://teem.sourceforge.net/nrrd/index.html

This module is a wrapper around the Nrrd library's function ```loadNrrd()```.

Upon opening, the ReadNrrd GUI defaults to reading a single file in the directory where the user's SCIRun executable resides, but also allows the user to navigate to the directory that the user sets for their SCIRUN_DATA environment variable through the Directory widget. The only supported file type is "*.nrrd".

If the user attempts to read in a file other than a SCIRun supported file type, or uses an incorrect file format, an error message will be logged on the SCIRun module. Clicking the **Set** button will load the file. Clicking the **Execute** button will send the loaded field downstream if other modules are connected to the ReadField module's output port.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
