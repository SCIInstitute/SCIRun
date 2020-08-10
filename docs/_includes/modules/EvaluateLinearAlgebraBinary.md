---
title: EvaluateLinearAlgebraBinary
category: moduledocs
module:
  category: Math
  package: SCIRun
tags: module
---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

This module performs one of a number of selectable matrix operations using the two input matrices.

**Detailed Description**

One of a number of binary matrix operations can be selected from the GUI and is then applied to the two input matrices to produce a new matrix that is then sent to the output port. The operations are X, +, Normalize, Select Rows, Select Columns, or Function.

**"A X B"** does a matrix multiply of the two matrices. The number of rows in the first matrix must match the number of columns in the second matrix. The output matrix has the same number of columns as the first matrix and the same number of rows as the second.

**"A + B"** performs a matrix addition. The two input matrices must be the same size. The elements are added in a piecewise fashion. This is equivalent to selecting "Function" and using 'x+y' as the function description

**"Normalize A to B"** computes the min and max values of the second matrix, and then linearly transforms all of the elements in the first matrix so that they fall within the range of those min and max values.

**"Select Rows"** uses the values in the B input matrix as row indices to extract from the A input matrix. The B input matrix must be a Column Matrix containing valid row index values into the A matrix. For example, if the B matrix contains [3 5] and the A matrix is 100x200, then the resulting matrix will be 2x200 and contain rows 3 and 5 from the A matrix.

**"Select Columns"** uses the values in the B input matrix as column indices to extract from the A input matrix. The B input matrix must be a ColumnMatrix containing the valid column index values into the A matrix. For example, if the B matrix contained [3 5] and the A matrix is 100x200, then the resulting matrix will be 100x2 and contain columns 3 and 5 from the A matrix.

**"Function"** allows an arbitrary function of two variables to be evaluated for each number pair in the two input matrices. This requires that the two matrices are the same size. The variable representing the element from the A matrix is 'x', and the variable for the element from the B matrix is 'y'. The function is specified using SCIRun's simple function parser. There are a number of mathematical functions available for use. They are:

**Operators:**

  * +:Add two numbers:

```
      4+3 = 7
```

  * -:Subtract one number from another:

```
      4-3 = 1
```

  * Multiply two numbers:

```
      4*3 = 12
```

  * Divide one number from another:

```
      12/3 = 4
```

  * sin:Sine of a number in **radians**:

```
      sin(x)
```

  * cos:Cosine of a number in **radians**:

```
      cos(x)
```

  * sqrt:Square root of a number:

```
      sqrt(4) = 2
```

  * sqr:Square of a number:

```
      sqr(2) = 4
```

  * ln:Natural logarithm of a number:

```
      ln(x)
```

  * exp:e raised to the nth power:

```
      exp(ln(x)) = x
```

  * log:Log base 10 of a number:

```
      log(100) = 2
```

  * abs:Absolute value of a number:

```
      abs(-3) = 3
```

  * pow:One number raised to the power of another:

```
      pow(3, 2) = 9
```

  * random:Return a uniform random number between 0 and 1:

```
      random()
```

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
