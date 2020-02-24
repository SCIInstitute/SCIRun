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


# CMakeLists.txt for Dataflow/Modules/Visualization

SET(Modules_Legacy_Visualization_SRCS
  #CreateScaleBar.cc
  #CreateLightForViewer.cc
  #ChooseColorMap.cc
  #CreateAndEditColorMap.cc
  #CreateAndEditColorMap2D.cc
  #CreateViewerAxes.cc
  #CreateViewerClockIcon.cc
  #CreateStandardColorMaps.cc
  #ExtractIsosurface.cc
  #ConvertNrrdsToTexture.cc
  #ConvertFieldsToTexture.cc
  #RescaleColorMap.cc
  #ShowColorMap.cc
  #ShowFieldGlyphs.cc
  #ShowLinePath.cc
  #ShowMatrix.cc
  #ShowMeshBoundingBox.cc
  #ShowPointPath.cc
  GenerateStreamLines.cc
  #GenerateStreamLinesWithPlacementHeuristic.cc
  #ShowTextureSlices.cc
  #ShowTextureSurface.cc
  #ShowTextureVolume.cc
  #ShowString.cc
  ShowAndEditDipoles.cc
)

SET(Modules_Legacy_Visualization_HEADERS
  GenerateStreamLines.h
  ShowAndEditDipoles.h
  share.h
)

SCIRUN_ADD_LIBRARY(Modules_Legacy_Visualization
  ${Modules_Legacy_Visualization_SRCS}
  ${Modules_Legacy_Visualization_HEADERS})

TARGET_LINK_LIBRARIES(Modules_Legacy_Visualization
  Dataflow_Network
  #Dataflow_Widgets
  #Dataflow_GuiInterface
  #Dataflow_TkExtensions
  Core_Algorithms_Legacy_Fields
  Algorithms_Base
  #Core_Algorithms_Visualization
  #Core_Algorithms_Geometry
  #Core_Algorithms_Converter
  Core_Basis
  Core_Datatypes
  #Core_Datatypes_Legacy_Bundle
  #Core_Exceptions_Legacy
  #Core_Geom
  Core_Geometry_Primitives
  #Core_GeomInterface
  #Core_ImportExport
  Core_Math
  #Core_Parser
  #Core_Persistent
  #Core_Thread
  Core_Util_Legacy
  Graphics_Widgets
  Graphics_Datatypes
  #${SCI_TEEM_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Legacy_Visualization)
ENDIF(BUILD_SHARED_LIBS)
