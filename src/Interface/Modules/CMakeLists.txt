
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
  Math/AppendMatrix.ui
  Math/CreateMatrix.ui
  Math/EvaluateLinearAlgebraBinary.ui
  Math/EvaluateLinearAlgebraUnary.ui
  Math/ReportMatrixInfo.ui
  ModuleDialogBasic.ui
  DataIO/ReadMatrix.ui
  Basic/ReceiveScalar.ui
  Basic/SendScalar.ui
  DataIO/WriteMatrix.ui
  Render/ViewScene.ui
  String/CreateString.ui
  Visualization/MatrixAsVectorField.ui
  Visualization/ShowString.ui
)

SET(Interface_Modules_HEADERS
  Math/AppendMatrixDialog.h
  Math/CreateMatrixDialog.h
  Math/EvaluateLinearAlgebraBinaryDialog.h
  Math/EvaluateLinearAlgebraUnaryDialog.h
  Math/ReportMatrixInfoDialog.h
  Basic/SendScalarDialog.h
  ModuleDialogBasic.h
  ModuleDialogFactory.h
  ModuleDialogGeneric.h
  DataIO/ReadMatrixDialog.h
  Basic/ReceiveScalarDialog.h
  Share.h
  DataIO/WriteMatrixDialog.h
  Render/ViewScene.h
  String/CreateStringDialog.h
  Visualization/MatrixAsVectorFieldDialog.h
  Visualization/ShowStringDialog.h
)

SET(Interface_Modules_SOURCES 
  Math/AppendMatrixDialog.cc
  Math/CreateMatrixDialog.cc
  Math/EvaluateLinearAlgebraBinaryDialog.cc
  Math/EvaluateLinearAlgebraUnaryDialog.cc
  Math/ReportMatrixInfoDialog.cc
  ModuleDialogBasic.cc
  ModuleDialogFactory.cc
  ModuleDialogGeneric.cc
  DataIO/ReadMatrixDialog.cc
  Basic/ReceiveScalarDialog.cc
  Basic/SendScalarDialog.cc
  DataIO/WriteMatrixDialog.cc
  Render/ViewScene.cc
  String/CreateStringDialog.cc
  Visualization/MatrixAsVectorFieldDialog.cc
  Visualization/ShowStringDialog.cc
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
  Algorithms_Math
  Algorithms_DataIO
  Modules_Basic
  Modules_String
  Modules_Math
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES})

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules)
ENDIF(BUILD_SHARED_LIBS)
  
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})