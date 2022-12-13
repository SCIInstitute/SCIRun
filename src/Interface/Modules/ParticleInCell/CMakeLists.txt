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


SET(Interface_Modules_ParticleInCell_FORMS
  ElectroStaticDialog.ui
  GravitySimulationDialog.ui
  PIConGPUDialog.ui
  PIConGPUReaderDialog.ui
  ParticleReaderDialog.ui
  ScalarMeshReaderDialog.ui
  VectorMeshReaderDialog.ui
)

SET(Interface_Modules_ParticleInCell_HEADERS
  ElectroStaticDialog.h
  GravitySimulationDialog.h
  PIConGPUDialog.h
  PIConGPUReaderDialog.h
  ParticleReaderDialog.h
  ScalarMeshReaderDialog.h
  VectorMeshReaderDialog.h
  share.h
)

SET(Interface_Modules_ParticleInCell_SOURCES
  ElectroStaticDialog.cc
  GravitySimulationDialog.cc
  PIConGPUDialog.cc
  PIConGPUReaderDialog.cc
  ParticleReaderDialog.cc
  ScalarMeshReaderDialog.cc
  VectorMeshReaderDialog.cc
)

QT_WRAP_UI(Interface_Modules_ParticleInCell_FORMS_HEADERS "${Interface_Modules_ParticleInCell_FORMS}")
QT_WRAP_CPP(Interface_Modules_ParticleInCell_HEADERS_MOC "${Interface_Modules_ParticleInCell_HEADERS}")

SCIRUN_ADD_LIBRARY(Interface_Modules_ParticleInCell
  ${Interface_Modules_ParticleInCell_HEADERS}
  ${Interface_Modules_ParticleInCell_FORMS_HEADERS}
  ${Interface_Modules_ParticleInCell_HEADERS_MOC}
  ${Interface_Modules_ParticleInCell_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_ParticleInCell
  Modules_ParticleInCell
  Modules_Basic
  Interface_Modules_Base
  Modules_Legacy_Bundle
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_ParticleInCell)
ENDIF(BUILD_SHARED_LIBS)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
