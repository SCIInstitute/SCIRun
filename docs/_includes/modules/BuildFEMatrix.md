---
title: BuildFEMatrix
category: moduledocs
module:
  category: FiniteElements
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module sets up a mesh with conductivity tensors for bioelectric field problems.

**Detailed Description**

The module provides basic means for construction of the FE matrix with linear elements for bioelectric field problems (discretization of Poisson equation for volume conductor problem). 

The input field should be a Field (TriSurf, HexVol, or TetVol) of either Tensors or integers, with the data values defined at the elements (constant over each element). If it is a Field of Tensors, each Tensor indicates the local conductivity tensor at that location in the mesh. If it is a Field of Integers, then each integer is an index into a lookup table, where the table contains a set of conductivity tensors. 

When using the index/table approach, a **conductivity_table** property must be stored with the Field, where the conductivity table is of type pair<string,Tensor> (i.e. the individual material types can have descriptive names). The BioPSE::Modeling::ModifyConductivities module can be used to generate (as well as to modify) the conductivity_table property. The BioPSE::Forward::IndicesToTensors and BioPSE::Forward::TensorsToIndices modules are useful for converting between the index/table (Field of integer with a conductivity_table Propert) and the full Tensor (Field of Tensors) representations.

BioPSE::Algorithm::BuildFEMatrix::build_FEMatrix performs the actual computation of the matrix. Other modules or any other code may use the algorithm for other kinds of problems involving discretization of the Poisson equation by linear finite elements.

The discretization of the Poisson equation on the supplied mesh is performed by linear finite elements using Galerkin approach.


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
