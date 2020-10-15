---
title: GetFieldNodes
category: moduledocs
module:
  category: ChangeMesh
  package: SCIRun
tags: module
layout: null
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module loads in a field and returns its nodes in the form of a matrix.

**Detailed Description**

Input: a SCIRun field. Output: All the nodes of the input field, represented by an N*3 matrix. Each row gives the x-y-z coordinates of one node.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
