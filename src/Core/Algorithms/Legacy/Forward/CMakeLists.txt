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

# CMakeLists.txt for Packages/BioPSE/Core/Algorithms/NumApproximation

SET(Core_Algorithms_Legacy_Forward_SRCS
  BuildBEMatrixAlgo.cc
  InsertVoltageSourceAlgo.cc
  #CalcTMP.cc
)

SET(Core_Algorithms_Legacy_Forward_HEADERS
  BuildBEMatrixAlgo.h
  InsertVoltageSourceAlgo.h
  #CalcTMP.h
)

SCIRUN_ADD_LIBRARY(Core_Algorithms_Legacy_Forward 
  ${Core_Algorithms_Legacy_Forward_SRCS}
  ${Core_Algorithms_Legacy_Forward_HEADERS}
  )

TARGET_LINK_LIBRARIES(Core_Algorithms_Legacy_Forward
  Algorithms_Base
  Core_Datatypes
  Core_Datatypes_Legacy_Field
  Core_Geometry_Primitives
  Core_Math
  Core_Basis
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Algorithms_Legacy_Forward)
ENDIF(BUILD_SHARED_LIBS)
