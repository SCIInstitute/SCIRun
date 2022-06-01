---
title: InsertEnvironmentIntoBundle
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

Collects the current environment variables into a bundle.

**Detailed Description**

InsertEnvironmentIntoBundles reads all the environment variables used in the current SCIRun session and saves them into a bundle for use in the SCIRun Network. It saves each environment variable as a string with the same name as the variable and the contents of the value of the variable. This module is useful in scripting SCIRun because parameters can be set outside SCIRun and passed into SCIRun, e.g. paths to directories were data is stored. Use GetStringFromBundle to read the environment variables saved into the bundle.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
