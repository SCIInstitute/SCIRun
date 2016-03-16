
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

SET(Interface_Modules_Render_FORMS
  ViewScene.ui
  ViewSceneControls.ui
)

SET(Interface_Modules_Render_HEADERS
  share.h
  ViewScene.h
  ViewSceneControlsDock.h
  GLWidget.h
  GLContext.h
  GLContextPlatformCompatibility.h
  QtGLContext.h
  Screenshot.h
  ES/SRCamera.h
  ES/SRInterface.h
  ES/SRUtil.h
  ES/Core.h
  ES/CoreBootstrap.h
  ES/AssetBootstrap.h
  ES/comp/RenderBasicGeom.h
  ES/comp/StaticSRInterface.h
  ES/comp/StaticWorldLight.h
  ES/comp/StaticClippingPlanes.h
  ES/comp/LightingUniforms.h
  ES/comp/ClippingPlaneUniforms.h
  ES/systems/RenderBasicSys.h
  ES/systems/RenderTransBasicSys.h
  ES/systems/RenderTransText.h
)

SET(Interface_Modules_Render_SOURCES 
  ViewScene.cc
  ViewSceneControlsDock.cc
  GLWidget.cc
  GLContext.cc
  QtGLContext.cc
  Screenshot.cc
  ES/SRCamera.cc
  ES/SRInterface.cc
  ES/SRUtil.cc
  ES/Core.cc
  ES/CoreBootstrap.cc
  ES/Registration.cc
  ES/AssetBootstrap.cc
  ES/comp/LightingUniforms.cc
  ES/comp/ClippingPlaneUniforms.cc
  ES/systems/RenderBasicSys.cc
  ES/systems/RenderTransBasicSys.cc
  ES/systems/RenderTransText.cc
)

QT4_WRAP_UI(Interface_Modules_Render_FORMS_HEADERS ${Interface_Modules_Render_FORMS})
QT4_WRAP_CPP(Interface_Modules_Render_HEADERS_MOC ${Interface_Modules_Render_HEADERS})

SCIRUN_ADD_LIBRARY(Interface_Modules_Render 
  ${Interface_Modules_Render_HEADERS}
  ${Interface_Modules_Render_FORMS_HEADERS}
  ${Interface_Modules_Render_HEADERS_MOC}
  ${Interface_Modules_Render_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Render 
  Interface_Modules_Base
  Core_Application_Preferences
  Core_Application
  Core_Datatypes
  ${OPENGL_LIBRARIES}
  ${QT_OPENGL_LIBRARY}
  ${CPM_LIBRARIES}
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
  ${SCI_FREETYPE_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Render)
ENDIF(BUILD_SHARED_LIBS)
  
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SCIRUN_ADD_TEST_DIR(Tests)