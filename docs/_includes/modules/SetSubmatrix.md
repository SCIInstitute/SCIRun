---
title: SetSubmatrix
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

Redefines a specified region of an input matrix to new, user-defined values.

**Detailed Description**

This module will replace existing values within a matrix with new values defined by the user. The module has 3 input ports. The first input port reads in the matrix the user wishes to change. The second port reads in the values that the user wishes to place inside the existing matrix. The third port is optional and reads in a 2x1 or 1x2 matrix that defines at which row and column the replacement will begin. The first number in the third port input matrix defines the initiating row. The second defines the column. If the third port is left empty the module defaults to row 0 / column 0, but these values can be altered manually within the UI.

**Example 1:**

Input port 1 = 

```
       [1 9 -13; 20 5 -6]
```

Input port 2 = 

```
       [a b]
```

Input port 3 = 

```
       [1 1]
```

Output = 

```
       [1 9 -13; 20 a b]
```


**Example 2:**

Input port 1 = 

```
       [1 9 -13; 20 5 -6]
```

Input port 2 = 

```
       [a; b]
```

Input port 3 = 

```
       [0 1]
```

Output = 

```
       [1 a -13; 20 b -6]
```


Note: The size of the input matrix cannot be altered by the size and starting points of the replacement and position matrices. In other words, if the resulting matrix would have larger dimensions than the original input matrix, the module with throw and error.

{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
