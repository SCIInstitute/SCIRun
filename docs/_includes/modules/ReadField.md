---
title: ReadField
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

This module allows the user to load any of the SCIRun supported **Field** file types and then sets that field in the module's output port.

**Detailed Description**

Upon opening, the ReadField GUI defaults to reading a single file in the directory where the user's SCIRun executable resides, but also allows the user to navigate to the directory that the user sets for their SCIRUN_DATA environment variable through the Directory widget. The default file type is "*.fld". Other supported file types are listed in the Files of type widget; these are generally NRRD, Matlab and mesh types. Files generally contain point cloud, surface or volume geometry such as point cloud points, triangle surface points and elements, tetrahedral mesh points and elements etc.

A typical way to import data into SCIRun is to read in geometry, and then also files containing data, then apply the data using a module like {% include moduleLink.md moduleName='SetFieldData' %}.

If the user attempts to read in a file other than a SCIRun supported file type, or uses an incorrect file format, an error message will be logged on the SCIRun module. Clicking the **Set** button will load the file. Clicking the **Execute** button will send the loaded field downstream if other modules are connected to the ReadField module's output port.

The **Time Series** tab allows the user to load files numbered sequentially with a common basename with an optional delay. The playback controls allow the user to play through the files, or step forward and backwards.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
