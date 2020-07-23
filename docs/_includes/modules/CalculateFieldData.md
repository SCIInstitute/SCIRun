---
title: CalculateFieldData
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

This module calculates a new value for each value in the Field data based on a user defined function. This function is based on a series of variables that is available for each data location. Once the function is defined, the module will walk through each data value and apply the function.

**Detailed Description**

This module allows the computation of a new scalar, vector or tensor value for each data location in the Field. The user defined function can depend on a number of variables that are defined for each location:

#### Input Variables

  1. ```DATA```: This is the current value stored in the Field (either on the element or the node location).

  2. ```X,Y,Z```: Cartesian coordinates of the node or element (center of the element)

  3. ```POS```: Vector with Cartesian coordinates of the node or element

  4. ```A,B,C,...```: Input from additional Matrix ports. The input Matrix can have either 1 row or the same number of rows as there are values in the Field. In case the Matrix has one value this value is the same for each data location, in case it has multiple values the module iterates of the values in the same way it iterates over the data values of the Field. The Matrix input can have either 1 column, 3 columns, 6 or 9 columns. In case the Matrix has 1 column values are assumed to be scalar values, in case the Matrix has 3 columns it is assumed to contain vector values and in case it has either 6 or 9 columns it is assumed to be a tensor value. A 6 valued tensor is defined as xx, xy, xz, yy, yz, and zz.

  5. ```INDEX```: The index number of the element or node.

  6. ```SIZE```: The number of elements or nodes in the Field (depends on the input Field mesh type).

  7. ```ELEMENT```: Special access variable to access properties of the element. Currently only length, area, and volume are available to be called on this entity. In case one is iterating over the nodes, the node point is assumed to be the element, in case one is iterating of the elements, this variable is referring to the full element.

#### Output Variable

The output needs to be stored in the variable ```RESULT```.

#### Available Functions

A list of available functions is available in the GUI of the module. The ***Parser Help*** button brings up a list of available functions to do scalar/vector/tensor algebra and to view the functions that can be applied to the ```ELEMENT``` variable.

#### Input Ports

The first input is the Field whose data needs to be recalculated using a function. The second port is an optional port that allows the user to script the module with a user defined input function. This function will override the function entered in module GUI. The third and next ports are used to import a Matrix. The first port corresponds to Matrix A, the next to Matrix B and so on. These ports can be used to do algebra with values stored as a Matrix or can be used to enter scriptable scalar/vector/tensor values that can be defined elsewhere.

#### Output Port

The module has one output port that has the newly defined values.

#### Example Functions

Suppose one wants to set the data values to a certain value:

```
RESULT = 2;
```

This will set every data value inside the Field equal to the value 2. 

Similarly one can set the data value to a value specified inside the first Matrix on the input ports:

```
RESULT = A;
```

If the Matrix contains only one value each data point is set to that value, if it contains the same number of values as datalocations, it will map each value in the Matrix to one value in the Field.

One can as well query the positions of the data point:

```
RESULT = X+Y;
```

This will store X+Y in each data location.

This same module can be used as well to generate vector or tensor data:

```
RESULT = Vector(X,Y,cos(A));
```

This will take the X, Y, position and the cos applied to the values in the Matrix A to create a new vector.

One can reuse the value that are there as well:

```
RESULT = DATA+A+B*C;
```

#### Output Data Type

As the function is parsed using the compiler, the output type cannot be guessed by the module, hence it needs to be set by the user to the correct data type.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
