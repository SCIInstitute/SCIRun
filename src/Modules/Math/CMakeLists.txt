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


SET(Modules_Math_SRCS
  #ConvertMatrixType.cc
  #SelectSubMatrix.cc
  #GetSubmatrix.cc
  AppendMatrix.cc
  CreateGeometricTransform.cc
  ConvertMatrixToScalar.cc
  ConvertScalarToMatrix.cc
  CreateMatrix.cc
  CreateComplexMatrix.cc
  EvaluateLinearAlgebraUnary.cc
  EvaluateLinearAlgebraBinary.cc
  GetMatrixSlice.cc
  ReportMatrixInfo.cc
  ReportComplexMatrixInfo.cc
  ReportMatrixSliceMeasure.cc
  SolveLinearSystem.cc
  SolveComplexLinearSystem.cc
  ComputePCA.cc
  ConvertRealToComplexMatrix.cc
  ConvertComplexToRealMatrix.cc
  BooleanCompare.cc
  DisplayHistogram.cc
  BasicPlotter.cc
  AdvancedPlotter.cc
  ResizeMatrix.cc
  CreateStandardMatrix.cc
)

SET(Modules_Math_HEADERS
  #ConvertMatrixType.h
  #SelectSubMatrix.h
  #GetSubmatrix.h
  AppendMatrix.h
  CreateGeometricTransform.h
  ConvertMatrixToScalar.h
  ConvertScalarToMatrix.h
  CreateMatrix.h
  CreateComplexMatrix.h
  EvaluateLinearAlgebraUnary.h
  EvaluateLinearAlgebraBinary.h
  GetMatrixSlice.h
  ReportMatrixInfo.h
  ReportComplexMatrixInfo.h
  ReportMatrixSliceMeasure.h
  share.h
  SolveLinearSystem.h
  SolveComplexLinearSystem.h
  ComputePCA.h
  ConvertRealToComplexMatrix.h
  ConvertComplexToRealMatrix.h
  BooleanCompare.h
  DisplayHistogram.h
  BasicPlotter.h
  AdvancedPlotter.h
  ResizeMatrix.h
  CreateStandardMatrix.h
)

SCIRUN_ADD_LIBRARY(Modules_Math
  ${Modules_Math_HEADERS}
  ${Modules_Math_SRCS}
)

TARGET_LINK_LIBRARIES(Modules_Math
  Dataflow_Network
  Core_Datatypes
  Algorithms_Math
)

IF(BUILD_WITH_PYTHON)
  TARGET_LINK_LIBRARIES(Modules_Math
    Core_Python
    )
ENDIF()

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Math)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
