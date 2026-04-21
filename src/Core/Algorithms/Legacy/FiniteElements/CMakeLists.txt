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


# Headers of Core/Algorithms/Legacy/FiniteElements classes
SET(Core_Algorithms_Legacy_FiniteElements_HEADERS
  ApplyFEM/ApplyFEMVoltageSourceAlgo.h
  BuildMatrix/BuildTDCSMatrix.h
  BuildMatrix/BuildFEMatrix.h
  BuildRHS/BuildFEVolRHS.h
  Mapping/BuildFEGridMapping.h
  Mapping/BuildNodeLink.h
  BuildRHS/BuildFESurfRHS.h
)

# Sources of Core/Algorithms/Legacy/FiniteElements classes
SET(Core_Algorithms_Legacy_FiniteElements_SRCS
  #Periodic/DefinePeriodicBoundaries.cc
  ApplyFEM/ApplyFEMVoltageSourceAlgo.cc
  Mapping/BuildFEGridMapping.cc
  Mapping/BuildNodeLink.cc
  BuildMatrix/BuildFEMatrix.cc
  BuildMatrix/BuildTDCSMatrix.cc
  BuildRHS/BuildFEVolRHS.cc
  BuildRHS/BuildFESurfRHS.cc
)

SCIRUN_ADD_LIBRARY(Core_Algorithms_Legacy_FiniteElements
  ${Core_Algorithms_Legacy_FiniteElements_HEADERS}
  ${Core_Algorithms_Legacy_FiniteElements_SRCS}
)

TARGET_LINK_LIBRARIES(Core_Algorithms_Legacy_FiniteElements
  Core_Datatypes
#  Core_Util
#  Core_Exceptions
#  Core_Thread
   Core_Geometry_Primitives
#  Core_Algorithms_Fields
#  Core_Algorithms_Util
#  Core_Persistent
#  Core_Basis
   Core_Datatypes_Legacy_Field
#  ${SCI_TEEM_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Algorithms_Legacy_FiniteElements)
ENDIF(BUILD_SHARED_LIBS)
