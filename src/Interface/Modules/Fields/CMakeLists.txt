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


SET(Interface_Modules_Fields_FORMS
  CreateLatVol.ui
  BuildMappingMatrix.ui
  GetDomainBoundary.ui
  JoinFields.ui
  FairMesh.ui
  ReportFieldInfoDialog.ui
  SplitFieldByDomain.ui
  SplitFieldByConnectedRegionDialog.ui
  CreateFieldData.ui
  CalculateFieldData.ui
  InterfaceWithCleaver2.ui
  InterfaceWithTetGen.ui
  SetFieldDataDialog.ui
  SetFieldDataToConstantValue.ui
  MapFieldDataFromElemToNodeDialog.ui
  GetSliceFromStructuredFieldByIndices.ui
  MapFieldDataFromNodeToElemDialog.ui
  MapFieldDataFromSourceToDestinationDialog.ui
  resampleregularmesh.ui #TODO: fix case
  ProjectPointsOntoMesh.ui
  calculatedistancetofield.ui #TODO: fix case
  calculatedistancetofieldboundary.ui #TODO: fix case
  MapFieldDataOntoElems.ui
  ConvertIndicesToFieldData.ui
  ConvertMeshToPointCloudDialog.ui
  MapFieldDataOntoNodes.ui
  MapFieldDataOntoNodesRadialbasis.ui
  ClipFieldByFunction.ui
  GenerateSinglePointProbeFromField.ui
  GeneratePointSamplesFromFieldOrWidget.ui
  GeneratePointSamplesFromField.ui
  RefineMesh.ui
  ConvertFieldBasis.ui
  swapfielddatawithmatrixentries.ui
  EditMeshBoundingBox.ui
  ExtractSimpleIsosurface.ui
  registerwithcorrespondences.ui
  ClipVolumeByIsovalue.ui
  ChooseInput.ui
  convertfielddatatype.ui
  scalefieldmeshanddata.ui #TODO: fix case
  ConvertMatricesToMesh.ui
  ReportFieldGeometryMeasures.ui
  RefineTetMeshLocally.ui
  GetMeshQualityField.ui
  CleanupTetMeshDialog.ui
  CalculateInsideWhichFieldDialog.ui
  CalculateMeshCenterDialog.ui
  CreateImageDialog.ui
  GetCentroidsFromMeshDialog.ui
  CalculateMeshNodes.ui
  ApplyFilterToFieldData.ui
  CalculateFieldDataMetric.ui
  CalculateBundleDifference.ui
  ReportScalarFieldStats.ui
)

SET(Interface_Modules_Fields_HEADERS
  BuildMappingMatrixDialog.h
  CreateLatVolDialog.h
  CreateFieldDataDialog.h
  CalculateFieldDataDialog.h
  GetDomainBoundaryDialog.h
  JoinFieldsDialog.h
  FairMeshDialog.h
  ReportFieldInfoDialog.h
  SplitFieldByDomainDialog.h
  SplitFieldByConnectedRegionDialog.h
  InterfaceWithCleaver2Dialog.h
  SetFieldDataDialog.h
  SetFieldDataToConstantValueDialog.h
  MapFieldDataFromElemToNodeDialog.h
  MapFieldDataFromNodeToElemDialog.h
  MapFieldDataFromSourceToDestinationDialog.h
  ResampleRegularMeshDialog.h
  ProjectPointsOntoMeshDialog.h
  CalculateDistanceToFieldDialog.h
  CalculateDistanceToFieldBoundaryDialog.h
  GetSliceFromStructuredFieldByIndicesDialog.h
  MapFieldDataOntoElemsDialog.h
  MapFieldDataOntoNodesDialog.h
  MapFieldDataOntoNodesRadialbasisDialog.h
  GenerateSinglePointProbeFromFieldDialog.h
  ClipFieldByFunctionDialog.h
  RefineMeshDialog.h
  ConvertFieldBasisDialog.h
  ConvertMeshToPointCloudDialog.h
  share.h
  SwapFieldDataWithMatrixEntriesDialog.h
  EditMeshBoundingBoxDialog.h
  ConvertIndicesToFieldDataDialog.h
  ExtractSimpleIsosurfaceDialog.h
  RegisterWithCorrespondencesDialog.h
  GeneratePointSamplesFromFieldDialog.h
  GeneratePointSamplesFromFieldOrWidgetDialog.h
  ClipVolumeByIsovalueDialog.h
  ChooseInputDialog.h
  ScaleFieldMeshAndDataDialog.h
  ConvertFieldDataTypeDialog.h
  ConvertMatricesToMeshDialog.h
  ReportFieldGeometryMeasuresDialog.h
  RefineTetMeshLocallyDialog.h
  GetMeshQualityFieldDialog.h
  CleanupTetMeshDialog.h
  CalculateInsideWhichFieldDialog.h
  CalculateMeshCenterDialog.h
  CreateImageDialog.h
  GetCentroidsFromMeshDialog.h
  CalculateMeshNodesDialog.h
  InterfaceWithTetGenDialog.h
  ApplyFilterToFieldDataDialog.h
  CalculateFieldDataMetricDialog.h
  CalculateBundleDifferenceDialog.h
  ReportScalarFieldStatsDialog.h
)

SET(Interface_Modules_Fields_SOURCES
  BuildMappingMatrixDialog.cc
  CreateLatVolDialog.cc
  CreateFieldDataDialog.cc
  CalculateFieldDataDialog.cc
  ConvertFieldBasisDialog.cc
  GetDomainBoundaryDialog.cc
  JoinFieldsDialog.cc
  FairMeshDialog.cc
  ReportFieldInfoDialog.cc
  SplitFieldByDomainDialog.cc
  SplitFieldByConnectedRegionDialog.cc
  InterfaceWithCleaver2Dialog.cc
  SetFieldDataDialog.cc
  SetFieldDataToConstantValueDialog.cc
  MapFieldDataFromElemToNodeDialog.cc
  MapFieldDataFromNodeToElemDialog.cc
  MapFieldDataFromSourceToDestinationDialog.cc
  ResampleRegularMeshDialog.cc
  GetSliceFromStructuredFieldByIndicesDialog.cc
  ProjectPointsOntoMeshDialog.cc
  GenerateSinglePointProbeFromFieldDialog.cc
  CalculateDistanceToFieldDialog.cc
  CalculateDistanceToFieldBoundaryDialog.cc
  MapFieldDataOntoElemsDialog.cc
  MapFieldDataOntoNodesDialog.cc
  MapFieldDataOntoNodesRadialbasisDialog.cc
  ClipFieldByFunctionDialog.cc
  SwapFieldDataWithMatrixEntriesDialog.cc
  RefineMeshDialog.cc
  EditMeshBoundingBoxDialog.cc
  ConvertIndicesToFieldDataDialog.cc
  ConvertMeshToPointCloudDialog.cc
  ExtractSimpleIsosurfaceDialog.cc
  RegisterWithCorrespondencesDialog.cc
  GeneratePointSamplesFromFieldDialog.cc
  GeneratePointSamplesFromFieldOrWidgetDialog.cc
  ClipVolumeByIsovalueDialog.cc
  ChooseInputDialog.cc
  ScaleFieldMeshAndDataDialog.cc
  ConvertFieldDataTypeDialog.cc
  ConvertMatricesToMeshDialog.cc
  ReportFieldGeometryMeasuresDialog.cc
  RefineTetMeshLocallyDialog.cc
  GetMeshQualityFieldDialog.cc
  CleanupTetMeshDialog.cc
  CalculateInsideWhichFieldDialog.cc
  CalculateMeshCenterDialog.cc
  CreateImageDialog.cc
  GetCentroidsFromMeshDialog.cc
  CalculateMeshNodesDialog.cc
  InterfaceWithTetGenDialog.cc
  ApplyFilterToFieldDataDialog.cc
  CalculateFieldDataMetricDialog.cc
  CalculateBundleDifferenceDialog.cc
  ReportScalarFieldStatsDialog.cc
)

QT_WRAP_UI(Interface_Modules_Fields_FORMS_HEADERS "${Interface_Modules_Fields_FORMS}")
QT_WRAP_CPP(Interface_Modules_Fields_HEADERS_MOC "${Interface_Modules_Fields_HEADERS}")

SCIRUN_ADD_LIBRARY(Interface_Modules_Fields
  ${Interface_Modules_Fields_HEADERS}
  ${Interface_Modules_Fields_FORMS_HEADERS}
  ${Interface_Modules_Fields_HEADERS_MOC}
  ${Interface_Modules_Fields_SOURCES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Fields
  Algorithms_Field
  Modules_Fields
  Interface_Modules_Base
  Modules_Legacy_Fields
  Modules_Basic
  ${SCI_BOOST_LIBRARY}
  ${QT_LIBRARIES}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Fields)
ENDIF(BUILD_SHARED_LIBS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../Base/CustomWidgets/CTK)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
