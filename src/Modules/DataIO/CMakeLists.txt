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


SET(Modules_DataIO_SRCS
  ReadField.cc
  ReadBundle.cc
  ReadMatrixClassic.cc
  WriteField.cc
  WriteG3D.cc
  WriteMatrix.cc
  ReadFile.cc
)

SET(Modules_DataIO_HEADERS
  GenericReader.h
  GenericWriter.h
  ReadField.h
  ReadBundle.h
  ReadMatrixClassic.h
  WriteField.h
  WriteG3D.h
  WriteMatrix.h
  ReadFile.h
)

SCIRUN_ADD_LIBRARY(Modules_DataIO
  ${Modules_DataIO_HEADERS}
  ${Modules_DataIO_SRCS}
)

TARGET_LINK_LIBRARIES(Modules_DataIO
  Dataflow_Network
  Core_Datatypes
  Core_Datatypes_Legacy_Field
  Core_Datatypes_Legacy_Bundle
  Algorithms_DataIO
  Core_Algorithms_Legacy_DataIO
  Core_ImportExport
  ${SCI_BOOST_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_DataIO)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
