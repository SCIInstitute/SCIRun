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


# CMakeLists.txt for Dataflow/Modules/DataIO

SET(Dataflow_Modules_DataIO_SRCS
  ReadBundle.cc
  WriteBundle.cc
  ReadColorMap2D.cc
  WriteColorMap2D.cc
  ReadColorMap.cc
  WriteColorMap.cc
  ReadField.cc
  WriteField.cc
  ReadFieldArray.cc
  WriteFieldArray.cc
  ReadMatrix.cc
  WriteMatrix.cc
  ReadPath.cc
  WritePath.cc
  ReadString.cc
  WriteString.cc
  StreamMatrixFromDisk.cc
  StreamACQFileFromDisk.cc
)

IF (HAVE_HDF5)
  SET(Dataflow_Modules_DataIO_SRCS ${Dataflow_Modules_DataIO_SRCS}
    ReadHDF5File.cc
    WriteHDF5DumpFile.cc)
ENDIF(HAVE_HDF5)

SCIRUN_ADD_LIBRARY(Dataflow_Modules_DataIO ${Dataflow_Modules_DataIO_SRCS})

TARGET_LINK_LIBRARIES(Dataflow_Modules_DataIO
  Dataflow_Network
  Dataflow_GuiInterface
  Dataflow_TkExtensions
  Core_Algorithms_Util
  Core_Algorithms_DataIO
  Core_Algorithms_DataStreaming
  Core_Basis
  Core_Datatypes
  Core_Exceptions
  Core_ImportExport
  Core_Geom
  Core_GeomInterface
  Core_Geometry
  Core_Persistent
  Core_Thread
  Core_Util
  Core_Volume
  ${TK_LIBRARY}
  ${GL_LIBRARY}
  ${M_LIBRARY}
  ${SCI_TEEM_LIBRARY}
  ${SLIVR_LIBRARY}
)

IF (HAVE_HDF5)
  TARGET_LINK_LIBRARIES(Dataflow_Modules_DataIO
    ${HDF5_LIBRARY})
ENDIF(HAVE_HDF5)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Dataflow_Modules_DataIO)
ENDIF(BUILD_SHARED_LIBS)
