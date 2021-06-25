# CalculateMeshNodes

This module allows the computation of a new position for each node in the input field. The user defined function can depend on a number of variables

**Detailed Description**

**Overview**

#### Input Variables

  1. ```DATA```: This is the current value stored in the field (either on the element or the node location). The value is only available if the data is located on the nodes.

  2. ```X,Y,Z```: Cartesian coordinates of the node or element (center of the element)

  3. ```POS```: Vector with Cartesian coordinates of the node or element

  4. ```A,B,C,...```: Input from additional matrix ports. The input matrix can have either 1 row or the same number of rows as there are values in the field. In case the matrix has one value this value is the same for each data location, in case it has multiple values the module iterates of the values in the same way it iterates over the data values of the field. The matrix input can have either 1 column, 3 columns, 6 or 9 columns. In case the matrix has 1 column values are assumed to be scalar values, in case the matrix has 3 columns it is assumed to contain vector values and in case it has either 6 or 9 columns it is assumed to be a tensor value (A 6 valued tensor is defined as xx, xy, xz, yy, yz, and zz).

  5. ```INDEX```: The index number of the element or node.

  6. ```SIZE```: The number of elements or nodes in the Field (depends on the input Field mesh type).

#### Output Variable

  1. ```NEWPOS```: The output needs to be stored in the variable NEWPOS.

#### Available Functions

A list of available functions is available in the GUI of the module. The [Parser Help](../../user_doc/parserhelp.md) button brings up a list of available functions to do scalar/vector/tensor algebra.

#### Input Ports

The first input is the Field whose node positions need to be recalculated using a function. The second port is an optional port that allows the user to script the module with a user defined input function. This function will override the function given in the GUI of the module. The third and next ports are used to import a matrix. The first port corresponds to matrix A, the next to matrix B and so on. These ports can be used to do algebra with values stored as a matrix or can be used to enter scriptable scalar/vector/tensor values that can be defined elsewhere.

#### Output Port

The module has one output port that has the newly defined values.

#### Output Data Type

As the function is parsed using the compiler, the output type cannot be guessed by the module, hence it needs to be set by the user to the correct data type.
