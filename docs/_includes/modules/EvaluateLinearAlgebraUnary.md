---
title: EvaluateLinearAlgebraUnary
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

Performs one of a number of selectable unary matrix operations to the input matrix.

**Detailed Description**

One of a number of unary matrix operations can be selected from the GUI and is then applied either to the whole matrix or to each element in the matrix, depending upon which operation is selected. The operations are Round, Ceil, Floor, Normalize, Transpose, Sort, Subtract Mean, or Function.

**Round**, **Ceil**, and **Floor** are used to convert each element in the input matrix into an integer value.

**Normalize** linearly transforms each element such that they all end up between 0 and 1.

**Transpose** constructs a new transpose matrix containing the same elements as the input matrix. For example, if the input matrix was of size 4x20, the output matrix will be of size 20x4.

**Sort** does an insertion sort on the elements of the input matrix. The elements will be sorted such that the smallest element will be at the top of the matrix and the largest element will be at the end. If the matrix is not a row or column matrix, the matrix is sorted in a row-scanline order. For example, in a 2x30 matrix, the first row would contain the lower half of the values and the second row would contain the upper half.

**Subract Mean** computes the mean value of the matrix, and then subtracts that value from each element in the matrix.

**Function** allows an arbitrary function to be evaluated for each element in the matrix. The current value is represented as the variable 'x'. For instance, the default 'x+10' function adds 10 to each element in the matrix. A function of just '10' would set each element in the matrix to be 10. The function is specified using SCIRun's simple function parser.

There are a number of mathematical functions available for use:

#### Operators

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
