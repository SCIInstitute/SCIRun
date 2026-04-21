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

# CMakeLists.txt for Packages/Teem/Dataflow/Modules/Tend

SET(Modules_Legacy_Teem_Tend_SRCS
  #TendAnscale.cc
  #TendAnvol.cc
  #TendBmat.cc
  #TendEpireg.cc
  #TendEstim.cc
  #TendEval.cc
  #TendEvalClamp.cc
  #TendEvalPow.cc
  #TendEvec.cc
  #TendEvecRGB.cc
  TendFiber.cc
  #TendNorm.cc
  #TendPoint.cc
  #TendSim.cc
  #TendSten.cc
  #TendAnplot.cc
  #TendAnhist.cc
  #TendSlice.cc
  #TendEvalAdd.cc
  #TendEvq.cc
  #TendExpand.cc
  #TendMake.cc
  #TendSatin.cc
  #TendShrink.cc
)

SET(Modules_Legacy_Teem_Tend_HEADERS
  TendFiber.h
  share.h
)

SCIRUN_ADD_LIBRARY(Modules_Legacy_Teem_Tend
  ${Modules_Legacy_Teem_Tend_SRCS}
  ${Modules_Legacy_Teem_Tend_HEADERS}
)

TARGET_LINK_LIBRARIES(Modules_Legacy_Teem_Tend
  Core_Datatypes
  Core_Datatypes_Legacy_Nrrd
  Core_Geometry_Primitives
  Core_Persistent
  Dataflow_Network
  ${SCI_TEEM_LIBRARY}
  #${SCI_PNG_LIBRARY}
  #${SCI_ZLIB_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Legacy_Teem_Tend)
ENDIF(BUILD_SHARED_LIBS)
