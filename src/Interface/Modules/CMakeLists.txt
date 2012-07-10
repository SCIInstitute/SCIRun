
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

SET(Interface_Modules_FORMS
  ModuleDialogBasic.ui
  ReceiveScalar.ui
  SendScalar.ui
)

SET(Interface_Modules_HEADERS
  SendScalarDialog.h
  ModuleDialogBasic.h
  ModuleDialogGeneric.h
  ReceiveScalarDialog.h
  Share.h
)

SET(Interface_Modules_SOURCES 
  ModuleDialogBasic.cc
  ReceiveScalarDialog.cc
  SendScalarDialog.cc
)

QT4_WRAP_UI(Interface_Modules_FORMS_HEADERS ${Interface_Modules_FORMS})
QT4_WRAP_CPP(Interface_Modules_HEADERS_MOC ${Interface_Modules_HEADERS})

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_LIBRARY(Interface_Modules 
${Interface_Modules_HEADERS}
${Interface_Modules_FORMS_HEADERS}
${Interface_Modules_HEADERS_MOC}
${Interface_Modules_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules 
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES})

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules)
ENDIF(BUILD_SHARED_LIBS)
  
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})