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


ADD_DEFINITIONS(-DGLM_SWIZZLE -DGLM_FORCE_CXX03 -DGLM_FORCE_RADIANS)

IF(UNIX)
  ADD_DEFINITIONS(-Wno-missing-braces)
ENDIF()

SET(Interface_Modules_Render_FORMS
  ViewScene.ui
  ViewSceneControls.ui
  ViewOspraySceneConfig.ui
  OsprayViewer.ui
)

SET(Interface_Modules_Render_HEADERS_TO_MOC
  ViewScene.h
  ViewSceneManager.h
  ViewSceneUtility.h
  ViewSceneControlsDock.h
  ViewOspraySceneConfig.h
  GLWidget.h
  GLContext.h
  GLContextPlatformCompatibility.h
  QtGLContext.h
  Screenshot.h
  OsprayViewerDialog.h
)

SET(Interface_Modules_Render_HEADERS_NO_MOC
  share.h
  ES/SRCamera.h
  ES/SRInterface.h
  ES/SRUtil.h
  ES/Core.h
  ES/CoreBootstrap.h
  ES/AssetBootstrap.h
  ES/Registration.h
  ES/RendererInterface.h
  ES/RendererInterfaceFwd.h
  ES/RendererCollaborators.h
  ES/RendererInterfaceCollaborators.h
  ES/comp/RenderBasicGeom.h
  ES/comp/StaticWorldLight.h
  ES/comp/StaticClippingPlanes.h
  ES/comp/LightingUniforms.h
  ES/comp/ClippingPlaneUniforms.h
  ES/comp/RenderList.h
  ES/comp/SRRenderState.h
  ES/systems/RenderBasicSys.h
  ES/systems/RenderTransBasicSys.h
  ES/systems/RenderTransText.h
)

SET(Interface_Modules_Render_SOURCES
  ViewScene.cc
  ViewSceneManager.cc
  ViewSceneUtility.cc
  ViewSceneControlsDock.cc
  GLWidget.cc
  GLContext.cc
  QtGLContext.cc
  Screenshot.cc
  OsprayViewerDialog.cc
  ViewOspraySceneConfig.cc
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

QT_WRAP_UI(Interface_Modules_Render_FORMS_HEADERS "${Interface_Modules_Render_FORMS}")
QT_WRAP_CPP(Interface_Modules_Render_HEADERS_MOC "${Interface_Modules_Render_HEADERS_TO_MOC}")

SCIRUN_ADD_LIBRARY(Interface_Modules_Render
  ${Interface_Modules_Render_HEADERS_TO_MOC}
  ${Interface_Modules_Render_HEADERS_NO_MOC}
  ${Interface_Modules_Render_FORMS_HEADERS}
  ${Interface_Modules_Render_HEADERS_MOC}
  ${Interface_Modules_Render_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Render
  Interface_Modules_Base
  Core_Application_Preferences
  Core_Application
  ${OPENGL_LIBRARIES}
  ${QT_OPENGL_LIBRARY}
  ${SCI_SPIRE_LIBRARY}
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
  ${SCI_FREETYPE_LIBRARY}
  ${SCI_LODEPNG_LIBRARY}
  ${SCI_TNY_LIBRARY}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Render Qt5::OpenGL)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Render)
ENDIF(BUILD_SHARED_LIBS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SCIRUN_ADD_TEST_DIR(Tests)

IF(WITH_OSPRAY)
  ADD_SUBDIRECTORY(Ospray)
  TARGET_LINK_LIBRARIES(Interface_Modules_Render
    Interface_Modules_Render_Ospray
    ospray::ospray
  )
ENDIF()
