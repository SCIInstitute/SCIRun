---
title: CreateFieldData
category: moduledocs
module:
  category: ChangeFieldData
  package: SCIRun
tags: module
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module calculates a new value for each value in the field data based on a user defined function. This function is based on a series of variables that is available for each data location. Once the function is defined, the module will walk through each data value and apply the function.

**Detailed Description**

This module allows the computation of a new scalar, vector or tensor value for each data location in the field. This module is closely related to CalculateFieldData, however it does not require any data to be present in the field and the data already in the field is ignored. This module also allows the user to state whether he or she wants to calculate data at the elements or at the nodes of the mesh.

The user defined function can depend on a number of variables that are defined for each location:

##### Input Variables

  1. ```X,Y,Z```: Cartesian coordinates of the node or element (center of the element)

  2. ```POS```: Vector with cartesian coordinates of the node or element

  3. ```A,B,C,...```: Input from additional matrix ports. The input matrix can have either 1 row or the same number of rows as there are values in the field. In case the matrix has one value this value is the same for each data location, in case it has multiple values the module iterates of the values in the same way it iterates over the data values of the field. The matrix input can have either 1 column, 3 columns, 6 or 9 columns. In case the matrix has 1 column values are assumed to be scalar values, in case the matrix has 3 columns it is assumed to contain vector values and in case it has either 6 or 9 columns it is assumed to be a tensor value (A 6 valued tensor is defined as xx, xy, xz, yy, yz, and zz).

  4. ```INDEX```: The index number of the element or node.

  5. ```SIZE```: The number of elements or nodes in the field. (Depending on the input field type)

  6. ```ELEMENT```: Special access variable to access properties of the element. Currently only length, area, and volume are available to be called on this entity. In case one is iterating over the nodes, the node point is assumed to be the element, in case one is iterating of the elements, this variable is referring to the full element.

##### Output Variable

The output needs to be stored in the variable ```RESULT```.

##### Available Functions

A list of available functions is available in the GUI of the module. Press on the button available functions to obtain a full overview of the current available functions to do Scalar/Vector/Tensor algebra and to view the functions that can be applied to the ```ELEMENT``` variable.

##### Input Ports

The first input is the field whose data needs to be recalculated using a function. The second port is an optional port that allows the user to script the module with a user defined input function. This function will override the function given in the GUI of the module. The third and next ports are used to import a matrix. The first port corresponds to matrix A, the next to matrix B and so on. These ports can be used to do algebra with values stored as a matrix or can be used to enter scriptable scalar/vector/tensor values that can be defined elsewhere.

##### Output Port

The module has one output port that has the newly defined values.

##### Output Data Type

As the function is parsed using the compiler, the output type cannot be guessed by the module, hence it needs to be set by the user to the correct data type.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
