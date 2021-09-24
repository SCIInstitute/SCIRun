---
title: ConvertNrrdToField
category: moduledocs
module:
  category: Converters
  package: Teem
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module takes a Nrrd and puts the data into a field.

**Detailed Description**

Nrrd is both a library and a file format. The full documentation can be found at http://teem.sourceforge.net/nrrd/index.html

The Data Location parameter lets you choose where to set the data in the field.
The Field Type parameter determines the data type that the field is composed of.
The Convert Parity parameter either does not correct, inverts the current parity or converts to a right handed coordinate system(RHS).

**Note about RHS**
The Teem documentation is not specific on how to deal with RHS and LHS. Hence we interpret the information as following:
(1) if a patient specific orientation is given, we check the parity of the space directions and the parity of the objective and convert if needed, i.e. either coord parity or space parity is LHS, then we mirror.
(2) if ScannerXYZ is given, nothing is assumed about coord parity and space parity, as it is not clear what has been defined.
(3) in case SpaceLeft3DHanded is given, we assume space parity is LHS and coord parity is not of importance.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
