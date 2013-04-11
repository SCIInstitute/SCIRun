#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2009 Scientific Computing and Imaging Institute,
#  University of Utah.
# 
#  
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
# 
#  The above copyright notice and this permission notice shall be included
#  in all copies or substantial portions of the Software.
# 
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
#  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
#  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#  DEALINGS IN THE SOFTWARE.
#

# Sources of Core/Math/Tests classes

SET(Core_Datatypes_Tests_SRCS
  MatrixInitializerTest.cc
  MatrixOperationsTest.cc
  MatrixTypeConversionsTest.cc
  BlockMatrixTests.cc
  MatrixElementAccessorPerformanceTest.cc
  ColumnMatrixMisfitCalculatorTest.cc
  SparseRowMatrixFromMapTest.cc
  CollectMatricesAlgorithmTest.cc
)

SCIRUN_ADD_UNIT_TEST(Core_Datatypes_Tests ${Core_Datatypes_Tests_SRCS})

# DV Fix defining below libraries
TARGET_LINK_LIBRARIES(Core_Datatypes_Tests
  Core_Math
  Core_Algorithms_Math
  Core_Exceptions 
  Core_Util
  Core_Datatypes
  ${SCI_GTESTMAIN_LIBRARY}
  ${M_LIBRARY}
  ${DL_LIBRARY}
  ${LAPACK_LIBRARY}
  ${CBLAS_LIBRARY}
  ${FBLAS_LIBRARY})
