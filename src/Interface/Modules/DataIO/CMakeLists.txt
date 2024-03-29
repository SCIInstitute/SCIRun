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


SET(Interface_Modules_DataIO_FORMS
  ReadFieldDialog.ui
  ReadBundleDialog.ui
  ReadMatrixClassic.ui
  ReadNrrd.ui
  ReadFile.ui
  WriteFieldDialog.ui
  WriteMatrix.ui
  WriteBundle.ui
  ReadColorMapXml.ui
)

SET(Interface_Modules_DataIO_HEADERS
  ReadFieldDialog.h
  ReadBundleDialog.h
  ReadMatrixClassicDialog.h
  ReadNrrdDialog.h
  ReadFileDialog.h
  ReadColorMapXmlDialog.h
  WriteBundleDialog.h
  WriteFieldDialog.h
  WriteMatrixDialog.h
  share.h
)

SET(Interface_Modules_DataIO_SOURCES
  ReadFieldDialog.cc
  ReadBundleDialog.cc
  ReadNrrdDialog.cc
  ReadFileDialog.cc
  ReadMatrixClassicDialog.cc
  ReadColorMapXmlDialog.cc
  WriteBundleDialog.cc
  WriteFieldDialog.cc
  WriteMatrixDialog.cc
)

QT_WRAP_UI(Interface_Modules_DataIO_FORMS_HEADERS "${Interface_Modules_DataIO_FORMS}")
QT_WRAP_CPP(Interface_Modules_DataIO_HEADERS_MOC "${Interface_Modules_DataIO_HEADERS}")

SCIRUN_ADD_LIBRARY(Interface_Modules_DataIO
  ${Interface_Modules_DataIO_HEADERS}
  ${Interface_Modules_DataIO_FORMS_HEADERS}
  ${Interface_Modules_DataIO_HEADERS_MOC}
  ${Interface_Modules_DataIO_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_DataIO
  Algorithms_DataIO
  Modules_DataIO
  Modules_Legacy_Teem_DataIO
  Interface_Modules_Base
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES})

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_DataIO)
ENDIF(BUILD_SHARED_LIBS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
