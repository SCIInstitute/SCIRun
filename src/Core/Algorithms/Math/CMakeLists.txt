#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

SET(Algorithms_Math_SRCS
  ConvertMatrixType.cc
  SelectSubMatrix.cc
  AppendMatrix.cc
  EvaluateLinearAlgebraUnaryAlgo.cc
  EvaluateLinearAlgebraBinaryAlgo.cc
  ReportMatrixInfo.cc
  GetMatrixSliceAlgo.cc
  SolveLinearSystemWithEigen.cc
  LinearSystem/SolveLinearSystemAlgo.cc
  ParallelAlgebra/ParallelLinearAlgebra.cc
  AddKnownsToLinearSystem.cc
  BuildNoiseColumnMatrix.cc
  ComputeSVD.cc
  ColumnMisfitCalculator/ColumnMatrixMisfitCalculator.cc
  ComputePCA.cc
  CollectMatrices/CollectMatricesAlgorithm.cc
  ReportMatrixSliceMeasureAlgo.cc
)

SET(Algorithms_Math_HEADERS
  ConvertMatrixType.h
  SelectSubMatrix.h
  AppendMatrix.h
  EvaluateLinearAlgebraUnaryAlgo.h
  EvaluateLinearAlgebraBinaryAlgo.h
  ReportMatrixInfo.h
  GetMatrixSliceAlgo.h
  share.h
  SolveLinearSystemWithEigen.h
  LinearSystem/SolveLinearSystemAlgo.h
  ParallelAlgebra/ParallelLinearAlgebra.h
  AddKnownsToLinearSystem.h
  BuildNoiseColumnMatrix.h
  ComputeSVD.h
  ColumnMisfitCalculator/ColumnMatrixMisfitCalculator.h
  ComputePCA.h
  CollectMatrices/CollectMatricesAlgorithm.h
  ReportMatrixSliceMeasureAlgo.h
)

SCIRUN_ADD_LIBRARY(Algorithms_Math 
  ${Algorithms_Math_HEADERS}
  ${Algorithms_Math_SRCS}
)

TARGET_LINK_LIBRARIES(Algorithms_Math
  Core_Datatypes
  Core_Thread
  Algorithms_Base
  Core_Parser
  ${SCI_BOOST_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Algorithms_Math)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
