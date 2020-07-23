---
title: AddLinkedNodesToLinearSystem
category: moduledocs
module:
  category: Math
  package: SCIRun
tags: module
---

<link rel="stylesheet" href="../css/modest.css">

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

AddLinkedNodesToLinearSystem will link nodes in a finite element mesh and solve them as a single value.

**Detailed Description**

AddLinkedNodesToLinearSystem works with {% include moduleLink.md moduleName='AddKnownsToLinearSystem' %} and {% include moduleLink.md moduleName='SolveLinearSystem' %} to force nodes in a finite element mesh to solve to the same value. This module is useful in solving for regions with homogeneous field properties, such as with a perfect conductor in an electric field. The algorithm to do this involves combining rows in the stiffness matrix corresponding to the nodes to solve together. The module also creates a mapping Matrix that will map the results of the solved linear system to the original mesh.

#### Input Ports

This module takes 3 inputs. The first two are the NxN Matrix (LHS) and the right hand vector (RHS), which is a Nx1 Matrix, for a linear system (```Ax=b```) and correspond to the two outputs of the {% include moduleLink.md moduleName='AddKnownsToLinearSystem' %} module.

The third input is a matrix (LinkedNodes) representing the data marking the nodes to link on the same mesh used in the {% include moduleLink.md moduleName='BuildFEMatrix' %} module. The nodes should be marked with an integer value so that nodes to be linked will have the same value. Any number of linked node sets can be used by using a different value, i.e., independent sets of nodes to be linked must be marked 1,2,3 creating three regions of linked nodes. The remaining nodes should be marked NaN. This can be accomplished be using {% include moduleLink.md moduleName='MapFieldDataOntoNodes' %} and using the ***Default Outside Value of NaN*** option and a ***Maximum Distance*** of some value that is small relative to your mesh size. Then use GetFieldData to port the matrix into this module.

#### Output Ports

This module returns 3 outputs. The first two are the reduced Matrix (OutputLHS) and right hand vector (OutputRHS), which is a Nx1 Matrix, for a linear system to use in {% include moduleLink.md moduleName='SolveLinearSystem' %}. The third is a mapping Matrix (Mapping) to restore the data (solution vector from linear system solution) to the proper size. To do this, multiply the mapping matrix and the first output of {% include moduleLink.md moduleName='SolveLinearSystem' %}. The resulting vector can then be applied to the original mesh used in the calculation.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
