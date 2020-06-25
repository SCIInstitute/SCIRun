---
title: PrintMatrixIntoString
category: moduledocs
module:
  category: String
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module does a sprintf with input matrices into a new string.

**Detailed Description**

This module module takes in a formatted string (a string with %01d, %4.5f, %g etc) and takes the numbers of the input matrix and puts these formatted numbers into the string. The input matrix can be a scalar but can as well be a full dense matrix. The module takes each number out of the matrix and puts it in the formatted number string. For example in order to plot the contents of a 1x3 vector one may choose a formatstring of '%f %f %f\n', which will put all three value in the matrix in the formatted string. If there are more numbers in the matrix than in the format string then the current format string will be reused again and the format string is filled out from the first entry again. Hence, to get the contents of a 4x4 matrix in a comma seperated list one can choose a format string of '%5.5f, c'. This will apply this format string to each number in the matrix. In case multiple matrices are supplied, first all the values of the first matrix will be used, then all the numbers of the second matrix, etc. When there are more positions in the matrix where one can fill out a number then the rest will be padded with the value 0.0.

The format string to be used can be either entered in the GUI or on the first input port. If a string is found at the first input port, this one is automatically inserted in the widget on the GUI and is the one to be used.

**Note:** the format string may contain '%s', which will remain in the format string and is not touched by this module. To insert strings use {% include moduleLink.md moduleName='PrintStringIntoString' %} instead.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
