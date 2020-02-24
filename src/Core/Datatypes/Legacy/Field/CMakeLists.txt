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


SET(Core_Datatypes_Legacy_Field_HEADERS
  CastFData.h
  CurveMesh.h
  Field.h
  FieldFwd.h
  FieldIndex.h
  FieldInformation.h
  FieldIterator.h
  FieldRNG.h
  FieldVIndex.h
  FieldVIterator.h
  GenericField.h
  HexVolMesh.h
  ImageMesh.h
  LatVolMesh.h
  Mesh.h
  MeshSupport.h
  MeshTypes.h
  PointCloudMesh.h
  PrismVolMesh.h
  QuadSurfMesh.h
  ScanlineMesh.h
  share.h
  StructCurveMesh.h
  StructHexVolMesh.h
  StructQuadSurfMesh.h
  TetVolMesh.h
  TriSurfMesh.h
  VFData.h
  VFDataT.h
  VField.h
  VMesh.h
  VMeshShared.h
  VUnstructuredMesh.h
)

SET(Core_Datatypes_Legacy_Field_SRCS
  cd_templates.cc
  cd_templates_fields_0.cc
  cd_templates_fields_1.cc
  cd_templates_fields_1a.cc
  cd_templates_fields_2.cc
  cd_templates_fields_2a.cc
  cd_templates_fields_3.cc
  cd_templates_fields_3a.cc
  cd_templates_fields_4.cc
  cd_templates_fields_4a.cc
  cd_templates_fields_5.cc
  cd_templates_fields_5a.cc
  cd_templates_fields_6.cc
  cd_templates_fields_6a.cc
  cd_templates_fields_6b.cc
  CurveMesh.cc
  Field.cc
  FieldInformation.cc
  FieldRNG.cc
  HexVolMesh.cc
  ImageMesh.cc
  LatVolMesh.cc
  Mesh.cc
  PointCloudMesh.cc
  PrismVolMesh.cc
  QuadSurfMesh.cc
  ScanlineMesh.cc
  TetVolMesh.cc
  TriSurfMesh.cc
  VFData.cc
  VFDataT_0.cc
  VFDataT_1.cc
  VFDataT_2.cc
  VMesh.cc
)

SCIRUN_ADD_LIBRARY(Core_Datatypes_Legacy_Field
  ${Core_Datatypes_Legacy_Field_SRCS}
  ${Core_Datatypes_Legacy_Field_HEADERS}
)

TARGET_LINK_LIBRARIES(Core_Datatypes_Legacy_Field
  Core_Basis
  Core_Exceptions_Legacy
  Core_Geometry_Primitives
  Core_Math
  #Core_Persistent
  Core_Thread
  #Core_Thread_Legacy
  Core_Utils
  Core_Util_Legacy
  Core_Datatypes
  Core_Datatypes_Legacy_Base
  ${SCI_BOOST_LIBRARY}
  #${GL_LIBRARY}
  #${M_LIBRARY}
  #${CBLAS_LIBRARY}
  #${F_LIBRARY}
  #${SCI_TEEM_LIBRARY}
  #${SCI_PNG_LIBRARY}
  #${SCI_ZLIB_LIBRARY}
  #${SCI_SLIVR_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Datatypes_Legacy_Field)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)
