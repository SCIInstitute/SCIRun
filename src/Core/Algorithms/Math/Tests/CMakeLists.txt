#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2020 Scientific Computing and Imaging Institute,
#  University of Utah.
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


SET(Algorithms_Math_Tests_SRCS
  AppendMatrixTests.cc
  ReportMatrixInfoTests.cc
  EvaluateLinearAlgebraUnaryTests.cc
  EvaluateLinearAlgebraBinaryTests.cc
  ParallelLinearAlgebraTests.cc
  SolveLinearSystemWithEigenTests.cc
  SolveLinearSystemAlgoTests.cc
  SolveLinearSystemAlgoTestsParameterized.cc
  AddKnownsToLinearSystemTests.cc
  ConvertMatrixTypeTests.cc
  SelectSubMatrixTests.cc
  GetMatrixSliceAlgoTests.cc
  ComputePCAtest.cc
  ComputeSVDtest.cc
  CollectMatricesAlgorithmTest.cc
)

#SET(Engine_Network_Tests_HEADERS
#  NetworkEditorControllerTests_.h
#)

SCIRUN_ADD_UNIT_TEST(Algorithms_Math_Tests
  ${Algorithms_Math_Tests_SRCS}
)

TARGET_LINK_LIBRARIES(Algorithms_Math_Tests
  Algorithms_Math
  Algorithms_DataIO
  Core_Datatypes
  Testing_Utils
  gtest_main
  gtest
  gmock
)
