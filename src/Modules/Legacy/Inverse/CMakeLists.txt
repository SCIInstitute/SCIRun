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


# CMakeLists.txt for Dataflow/Modules/Inverse

SET(Modules_Legacy_Inverse_HEADERS
  SolveInverseProblemWithTikhonov.h
  SolveInverseProblemWithTikhonovSVD.h
  SolveInverseProblemWithTSVD.h
  BuildSurfaceLaplacianMatrix.h
  LCurvePlot.h
  share.h
)

SET(Modules_Legacy_Inverse_SRCS
  SolveInverseProblemWithTikhonov.cc
  SolveInverseProblemWithTikhonovSVD.cc
  SolveInverseProblemWithTSVD.cc
  BuildSurfaceLaplacianMatrix.cc
  LCurvePlot.cc
)

SCIRUN_ADD_LIBRARY(Modules_Legacy_Inverse
  ${Modules_Legacy_Inverse_HEADERS}
  ${Modules_Legacy_Inverse_SRCS}
)

TARGET_LINK_LIBRARIES(Modules_Legacy_Inverse
 Algorithms_Legacy_Inverse
 Dataflow_Network
 Core_Algorithms_Legacy_Geometry
 Core_Datatypes_Legacy_Field
 Algorithms_Base
 Core_Basis
 Core_Datatypes
 Core_Exceptions_Legacy
 Core_Geometry_Primitives
 Core_Math
 Core_Util_Legacy
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Legacy_Inverse)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
