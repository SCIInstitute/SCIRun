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


SET(Modules_Math_Tests_SRCS
  AppendMatrixTests.cc
  ReportMatrixInfoTests.cc
  EvaluateLinearAlgebraUnaryTests.cc
  EvaluateLinearAlgebraBinaryTests.cc
  MatrixCalculatorFunctionalTests.cc
  SolveLinearSystemTests.cc
  SolveMinNormLeastSqSystemTests.cc
  ConvertMatrixTypeTests.cc
  SelectSubMatrixTests.cc
  AddKnownsToLinearSystemTests.cc
  PortCachingUnitTest.cc
  ComputePCAtest.cc
  ComputeSVDtest.cc
  ConvertRealToComplexMatrixTests.cc
  ConvertComplexToRealMatrixTests.cc
)

#SET(Engine_Network_Tests_HEADERS
#  NetworkEditorControllerTests_.h
#)

SCIRUN_ADD_UNIT_TEST(Modules_Math_Tests
  ${Modules_Math_Tests_SRCS}
)

TARGET_LINK_LIBRARIES(Modules_Math_Tests
  Modules_Math
  Modules_Factory
  Algorithms_Math
  Core_Datatypes
  Dataflow_Network
  Dataflow_State
  Algorithms_Factory
  Modules_Render
  Modules_Visualization
  Engine_Network
  Testing_Utils
  Testing_ModuleTestBase
  gtest_main
  gtest
  gmock
)
