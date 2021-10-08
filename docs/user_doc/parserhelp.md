# Parser Help

## Parser Description


### Overview

The SCIRun parser is designed to give the user the ability to evaluate almost any expression at the elements of a matrix or field. The underlying engine of the parser assumes that every field or matrix can be represented by an array of scalars, vectors, or tensors. For example a field can be represented by an array of scalars for the field data and an array of vectors for the locations of the nodes. In order to maintain a reasonable performance while interpreting a user defined function, the engine will apply each mathematical operations to the full array at same time. The logic of the parser will allow the user to write several expression at the same time, which will optimized to remove any duplicate function calls and the operations will be untangled in a series of elementary operations that have to be executed.


### Using the parser

Each parser has several preloaded variables available to the user. These preloaded variables in SCIRun are always denoted with an upper case variable name. For example for the [CalculateFieldData](../modules/ChangeFieldData/CalculateFieldData.md) has the following variables available:

  `DATA, X,Y,Z, POS`

The variables can be used in the user to construct a new variable. For example, one can write the following equation:

  `MYDATA = sin(X)+cos(Y);`

This will assign the value of `sin(X) + cos(Y)` to `MYDATA`, where `X` and `Y` are the arrays containing the `X` and `Y` positions of the nodes. Note that to generate a new variable, no new variable type needs to be declared. The parser will automatically derive the type of the output variable. For example the following case will generate vector data:

  `MYDATA = vector(Y,X,2*Z);`

As the output of the expression is a vector `MYDATA` will now be an array of vectors instead of an array of scalars as in the previous example.

To assign any data to the output of a module, certain variables need to be assigned. For example in case of the [CalculateFieldData](../modules/ChangeFieldData/CalculateFieldData.md), the output variable is called `RESULT` and hence a second expression can be added to assign the newly created vector to the output:

  `MYDATA = vector(Y,X,2*Z); RESULT = MYDATA;`

Now this expression could have been simplified to one expression, which would result in the same operation:

  `RESULT = vector(Y,X,2*Z);`

From a performance perspective both would be evaluated equally fast as, copying operations are generally removed by the internal optimizer. However to increase user readability one can split an expression in as many pieces as one wants. For example, the following expression is a perfectly valid expression:

  `DX = X-3; DY = Y-4; DZ = Z - 2; RESULT = norm(vector(DX,DY,DZ));`

Hence the general grammar of the expression is:

 `VAR1 = function(...);` `VAR2 = function(...);` `VAR3 = function(...);` ... etc

The array parser generally allows two types of input, an array with one element, or an array with many elements. When combining multiple fields or matrices, the arrays with many elements are required to have the same number of elements and the math operations are done by taking corresponding elements from each array. However an array with one element is treated special, and its value is used for every element for the arrays that have many arrays. For example, assume matrix `A = [1]`, and `B = [1,2,3,4]`. We can now evaluate the following expression:

  `RESULT = A+B;`

In this case `RESULT` will be `[1+1,2+1,3+1,4+1]` and hence allows for scalar parameters to be added into the expressions.



### Available functions

The following basic operators have been implemented for Scalars, Vectors, and Tensors:

  `+`, `-`, `*`, `/`, `==`, `!=`, `!`, `&&`, `||`

In addition to these operators the following functions are available:

   - `add()`
   - `sub()`
   - `mult()`
   - `div()`
   - `exp()`
   - `log()`
   - `ln()`
   - `log2()`
   - `log10()`
   - `sin()`
   - `cos()`
   - `tan()`
   - `asin()`
   - `acos()`
   - `atan()`
   - `sinh()`
   - `cosh()`
   - `asinh()`
   - `acosh()`
   - `pow()`
   - `ceil()`
   - `floor()`
   - `round()`
   - `boolean()`
   - `norm()`
   - `isnan()`
   - `isfinite()`
   - `isinfinite()`
   - `select()`
   - `sign()`
   - `sqrt()`
   - `not()`
   - `inv()`
   - `abs()`
   - `and()`
   - `or()`
   - `eq()`
   - `neq()`



**For vectors the following functions are available:**

To construct a vector use the following function:

  `MYVECTOR = vector(X,Y,Z);`

To access the components of a vector use the following functions:

  `X = x(MYVECTOR);` `Y = y(MYVECTOR);` `Z = z(MYVECTOR);`

The following vector specific functions are available:

  - `dot()`
  - `cross()`
  - `normalize()`
  - `find_normal1()`
  - `find_normal2()`

**For tensors the following functions are available:**

To construct a tensor use the following functions:

  `MYTENSOR = tensor(XX,XY,XZ,YY,YZ,ZZ);`

  `MYTENSOR = tensor(SCALAR);`

  `MYTENSOR = tensor(EIGVEC1,EIGVEC2,EIGVAl1,EIGVAL2,EIGVAL3);`

To access the components of a tensor use the following functions:

  `XX = xx(MYTENSOR); XY = xy(MYTENSOR); XZ = xz(MYTENSOR);`

  `YY = yy(MYTENSOR); YZ = yz(MYTENSOR); ZZ = zz(MYTENSOR);`

  `EIGVEC1 = eigvec1(MYTENSOR); EIGVEC2 = eigvec2(MYTENSOR); EIGVEC3 = eigvec3(MYTENSOR)`

  `EIGVAL1 = eigval1(MYTENSOR); EIGVAL2 = eigval2(MYTENSOR); EIGVAL3 = eigval3(MYTENSOR);`



**The following tensor specific functions are available:**

  `quality()`, `trace`, `det()`, `frobenius()`, `frobenius2()`, `fracanisotropy()`

For converting any Scalar, Vector or Tensor in to a boolean, e.g. whether all components are equal to zero or not, the function `boolean()` provided:

  `RESULT = boolean(vector(X,0,Z));`

  `RESULT = boolean(vector(X,0,Z)) || boolean(tensor(4)*0);`

The `select()` function works like the C/C++ ternary `?:` operator:

  `RESULT = select(X>2,1,0);`

To insert a random number two functions are provided, `rand()` and `randv()`. Neither of them take any input. The first one just render a single random number and the second renders an array of the same size as the other arrays. For example to render uniform distributed noise between 0 and 2 for a field, use the following expression:

  `RESULT = 2*randv();`

If on the other hand all the values need to be the same, but one random value, use :

  `RESULT = rand();`

Finally, we have an object called element, which refers to the element of a mesh. The latter only exists as an input object, but allows the user to query, properties like element quality metrics and sizes. Currently the following functions are implemented:

  `center()`,`volume()`,`size()`,`length()`,`area()`,`dimension()`
