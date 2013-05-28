
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

PROJECT(QtExampleDiagram)

SET(QtExampleDiagram_SOURCES 
  Link.cc
  Node.cc
  DiagramWindow.cc
  propertiesdialog.cc
  main.cc
  PythonConsoleWidget.cc
)

SET(QtExampleDiagram_HEADERS
  Link.h
  Node.h
  DiagramWindow.h
  propertiesdialog.h
  PythonConsoleWidget.h
)

SET(QtExampleDiagram_FORMS propertiesdialog.ui)

SET(QtExampleDiagram_RESOURCES resources.qrc)

QT4_WRAP_CPP(QtExampleDiagram_HEADERS_MOC ${QtExampleDiagram_HEADERS})
QT4_WRAP_UI(QtExampleDiagram_FORMS_HEADERS ${QtExampleDiagram_FORMS})
QT4_ADD_RESOURCES(QtExampleDiagram_RESOURCES_RCC ${QtExampleDiagram_RESOURCES})

ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(diagram
${QtExampleDiagram_SOURCES}
${QtExampleDiagram_HEADERS}
${QtExampleDiagram_FORMS_HEADERS}
${QtExampleDiagram_HEADERS_MOC}
${QtExampleDiagram_RESOURCES_RCC})

TARGET_LINK_LIBRARIES(diagram 
  PythonAPI
  Core_Python
  log4cpp
  ${QT_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${SCI_BOOST_LIBRARY}
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PYTHON_INCLUDE_DIR}
)

ADD_SUBDIRECTORY(API)