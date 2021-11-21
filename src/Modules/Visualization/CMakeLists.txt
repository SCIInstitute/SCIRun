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


SET(Modules_Visualization_SRCS
  CreateStandardColorMap.cc
  ShowField.cc
  ShowFieldGlyphs.cc
  ShowFieldGlyphsPortHandler.cc
  ShowString.cc
  ShowColorMapModule.cc
  RescaleColorMap.cc
  TextBuilder.cc
  ShowFieldWithOspray.cc
  CreateTestingArrow.cc
  ShowMeshBoundingBox.cc
  ShowOrientationAxes.cc
)

SET(Modules_Visualization_HEADERS
  CreateStandardColorMap.h
  ShowField.h
  ShowFieldGlyphs.h
  ShowFieldGlyphsPortHandler.h
  ShowString.h
  ShowColorMapModule.h
  RescaleColorMap.h
  TextBuilder.h
  ShowFieldWithOspray.h
  CreateTestingArrow.h
  ShowMeshBoundingBox.h
  ShowOrientationAxes.h
  share.h
)

SCIRUN_ADD_LIBRARY(Modules_Visualization
  ${Modules_Visualization_HEADERS}
  ${Modules_Visualization_SRCS}
)

TARGET_LINK_LIBRARIES(Modules_Visualization
  Dataflow_Network
  Core_Datatypes
  Core_Datatypes_Mesh
  Core_Datatypes_Legacy_Field
  Core_Algorithms_Visualization
  Graphics_Glyphs
  Graphics_Datatypes
  Graphics_Widgets
  ${SCI_FREETYPE_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Visualization)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
