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


SET(Core_GeometryPrimitives_SRCS
  BBoxBase.cc
  BBox.cc
  OrientedBBox.cc
  CompGeom.cc
  Plane.cc
  Point.cc
  SearchGridT.cc
  Tensor.cc
  Transform.cc
  Vector.cc
)

SET(Core_GeometryPrimitives_HEADERS
  BBoxBase.h
  BBox.h
  OrientedBBox.h
  CompGeom.h
  GeomFwd.h
  Plane.h
  Point.h
  PointVectorOperators.h
  SearchGridT.h
  Tensor.h
  Transform.h
  Vector.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_Geometry_Primitives
  ${Core_GeometryPrimitives_HEADERS}
  ${Core_GeometryPrimitives_SRCS}
)

TARGET_LINK_LIBRARIES(Core_Geometry_Primitives
  Core_Math
  Core_Util_Legacy
  Core_Persistent
  ${SCI_ZLIB_LIBRARY}
  ${SCI_PNG_LIBRARY}
  ${SCI_TEEM_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Geometry_Primitives)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
