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


SET(Core_Math_SRCS
  CubicPWI.cc
  Gaussian.cc
  LinAlg.c
  LinearPWI.cc
  Mat.c
  MiscMath.cc
  MiscMath2.cc
  MusilRNG.cc
  PiecewiseInterp.cc
  TrigTable.cc
  fft.c
  Histogram.cc
)

SET(Core_Math_HEADERS
  CatmullRomSpline.h
  CubicPWI.h
  Gaussian.h
  LinAlg.h
  LinearPWI.h
  Mat.h
  MiscMath.h
  MiscMath2.h
  MusilRNG.h
  PiecewiseInterp.h
  TrigTable.h
  fft.h
  MiscMath.h
  Histogram.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_Math
  ${Core_Math_HEADERS}
  ${Core_Math_SRCS}
)

TARGET_LINK_LIBRARIES(Core_Math
  Core_Exceptions_Legacy
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Math)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
