
#
#  For more information, please see: http://software.sci.utah.edu
# 
#  The MIT License
# 
#  Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

IF(VTK_SUPPORT)
  FIND_PACKAGE(VTK REQUIRED)
  INCLUDE(${VTK_USE_FILE})
  SET(GUI_VTK_SOURCES
    RenderWindow.cc)
  SET(GUI_VTK_HEADERS
    RenderWindow.h)
  SET(GUI_VTK_LINK_LIBRARIES
    QVTK)
ENDIF(VTK_SUPPORT)

##################################################
# Set sources
##################################################


SET(SCIRunGUIPrototype_SOURCES 
  ModuleWidget.cc
  ModuleProxyWidget.cc
  scirunMain.cc 
  SCIRunMainWindow.cc
  Node.cc
  Port.cc
  Connection.cc
  NetworkEditor.cc
  NetworkEditorControllerGuiProxy.cc
  ${GUI_VTK_SOURCES}
)

SET(SCIRunGUIPrototype_HEADERS
  Logger.h
  ModuleWidget.h
  ModuleProxyWidget.h
  SCIRunMainWindow.h
  Node.h
  Connection.h
  Port.h
  NetworkEditor.h
  NetworkEditorControllerGuiProxy.h
  ${GUI_VTK_HEADERS}
)

SET(SCIRunGUIPrototype_FORMS
  Module.ui
  SCIRunMainWindow.ui
  RenderWindow.ui
)

SET(SCIRunGUIPrototype_RESOURCES
#Interface/Application/scirun-icon-small.ico
#Interface/Application/scirun-icon-large.ico
#Interface/Application/lin_agt_wrench-32.png
)

QT4_WRAP_UI(SCIRunGUIPrototype_FORMS_HEADERS ${SCIRunGUIPrototype_FORMS})
QT4_WRAP_CPP(SCIRunGUIPrototype_HEADERS_MOC ${SCIRunGUIPrototype_HEADERS})
QT4_ADD_RESOURCES(SCIRunGUIPrototype_RESOURCES_RCC ${SCIRunGUIPrototype_RESOURCES})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(SCIRunGUIPrototype
  ${SCIRunGUIPrototype_SOURCES}
  ${SCIRunGUIPrototype_HEADERS}
  ${SCIRunGUIPrototype_HEADERS_MOC}
  ${SCIRunGUIPrototype_FORMS_HEADERS}
  ${SCIRunGUIPrototype_RESOURCES_RCC})

TARGET_LINK_LIBRARIES(SCIRunGUIPrototype 
  Core_Dataflow_Network
  Engine_Network
  Engine_State
  Modules_Factory
  Interface_Modules
  ${GUI_VTK_LINK_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES}
  ${CMAKE_THREAD_LIBS}
)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
