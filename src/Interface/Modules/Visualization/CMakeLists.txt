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


SET(Interface_Modules_Visualization_FORMS
  CreateStandardColorMap.ui
  GenerateStreamLines.ui
  ShowField.ui
  ShowFieldGlyphs.ui
  ShowString.ui
  ShowColorMap.ui
  RescaleColorMap.ui
  ShowFieldWithOspray.ui
  ShowAndEditDipoles.ui
  ShowMeshBoundingBox.ui
  ShowOrientationAxes.ui
  ShowUncertaintyGlyphs.ui
  GeometryBuffer.ui
  ReportColorMapInfo.ui
)

SET(Interface_Modules_Visualization_HEADERS
  CreateStandardColorMapDialog.h
  GenerateStreamLinesDialog.h
  share.h
  ShowFieldDialog.h
  ShowFieldGlyphsDialog.h
  ShowStringDialog.h
  ShowColorMapDialog.h
  RescaleColorMapDialog.h
  ShowFieldWithOsprayDialog.h
  ShowAndEditDipolesDialog.h
  ShowMeshBoundingBoxDialog.h
  ShowOrientationAxesDialog.h
  ShowUncertaintyGlyphsDialog.h
  GeometryBufferDialog.h
  ReportColorMapInfoDialog.h
)

SET(Interface_Modules_Visualization_SOURCES
  CreateStandardColorMapDialog.cc
  GenerateStreamLinesDialog.cc
  ShowFieldDialog.cc
  ShowFieldGlyphsDialog.cc
  ShowStringDialog.cc
  ShowColorMapDialog.cc
  RescaleColorMapDialog.cc
  ShowFieldWithOsprayDialog.cc
  ShowAndEditDipolesDialog.cc
  ShowMeshBoundingBoxDialog.cc
  ShowOrientationAxesDialog.cc
  ShowUncertaintyGlyphsDialog.cc
  GeometryBufferDialog.cc
  ReportColorMapInfoDialog.cc
)

QT_WRAP_UI(Interface_Modules_Visualization_FORMS_HEADERS "${Interface_Modules_Visualization_FORMS}")
QT_WRAP_CPP(Interface_Modules_Visualization_HEADERS_MOC "${Interface_Modules_Visualization_HEADERS}")

SCIRUN_ADD_LIBRARY(Interface_Modules_Visualization
  ${Interface_Modules_Visualization_HEADERS}
  ${Interface_Modules_Visualization_FORMS_HEADERS}
  ${Interface_Modules_Visualization_HEADERS_MOC}
  ${Interface_Modules_Visualization_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Visualization
  Modules_Visualization
  Core_Algorithms_Legacy_Fields
  Interface_Modules_Base
  Core_Application
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Visualization)
ENDIF(BUILD_SHARED_LIBS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
