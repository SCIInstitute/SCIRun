---
title: BooleanCompare
category: moduledocs
module:
  category: FlowControl
  package: SCIRun
tags: module

---

# {{ page.title }}

## Category

**{{ page.module.category }}**

## Description

### Summary

Compare one or two matrices using boolean operators, and send different outputs, or quit SCIRun,  depending on the result of the boolean comparison.   

**Detailed Description**

The BooleanCompare module will evaluate one or two input matrices and return different results based ont the evaluation.  The module will evaluate either the values, size, or norm of each of the matrices with boolean operations, and can return any one of three inputs, as well as quit SCIRun. The inputs will be compared element by element based on the criteria chosen in the Condition option and perform the actions chosen in the Then option if true and Else option if false.  Module also send the result of the comparison as the seond output matrix.  

*Inputs* 
- MatrixA - (Required) First matrix for comparison.
- MatrixB - (Optional) Second matrix for comparison.  If this port is not used, most of the comparsion options will not be possible.  
PossibleOutput - (Optional) Possible matrix to send as an output to the module.  

*Outputs*
- OutputMatrix - The output matrix of the module.  This will be identical to one of the three inputs of the module, or will be null depending on the inputs and options of the module. 
- BooleanResult - Matrix of 1x1 containing either 1 or 0 indicating the result  of the boolean comparison performed by the module. 

*Options*

Any combination of the values (default), size, and norm (except a size and norm) of the first two input matrices can be compared as long as the resulting size of the objects are the same.  The size of one matrix can be compared to the values of the other matrix if it has two entries. Likewise, the norm of one matrix can only be compared to the value of another matrix if it is 1x1.  

The condition option sets the method of comparing the input matrices.  If the operator is true, then the module will result in the action chosen in the Then_Option menu, and if false it will perform the action selected in the Else_Option menu.  Operator options are: 
- 'A is non-zero' - (default) true if there are any non-zero entries of A .
- 'A and B are non-zero (and)' - true if any corresponding entries of A and B are both non-zero.
- 'Either A or B is non-zero (or)' - ture if either A or B has non-zero entries 
- 'A is greater than or equal to B (>=)' -  true if entries of A are greater than or equal to B's.'  
- 'A is greater than B (>)' - true if entries of A are greater than B's.  
- 'A is equal to B (==)' -  true if all the entries of A are the same as B.
- 'A is less than B (<)' -  true if entries of A are less than B's.
- 'A is less than or equal to B (<=)' - true if entries of A are less than or equal to B's.

There can be ambiguous scenarios with the greater or equal, greater, less, less or equal options in the module if the input matrices have a size of more than one.  In these cases, ie, if there are a set of entries which are both less than and greater than, the number of entries and the magnitude of the difference will determine which input is greater or less.  Consider comparing the matrix norms to avoid ambiguity of the results.  If there is no second matrix input, the first matrix can only be evaluated by non-zero entries (or size or norm) of the matrix.  

The results options establishes the output of the module based on the result of the comparison.  If the condition returns true, the action indicated in the Then option will be performed,  If false, the Else option will be performed.  Possible actions are:
- 'Return First Input' - (Then default) returns the first matrix input.
- 'Return Second Input' - returns the second matrix input.
- 'Return Third Input' -  returns the third matrix input.
- 'Return null' - (Else default) returns an emplty matrix handle.
- 'Quit SCIRun' -  Quit the current instance of SCIRun.  

If the second or third inputs are selected for output and no matrix is input into these ports, the module will return null.  


{% capture url %}{% include url.md %}{% endcapture %}
{{ url }}
