#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

# CMakeLists.txt for Dataflow/Modules/Fields

SET(Modules_Legacy_Fields_HEADERS
  RefineMesh.h
  ClipVolumeByIsovalue.h
  CreateLatVol.h
  GetFieldBoundary.h
  BuildMappingMatrix.h
  BuildMatrixOfSurfaceNormals.h
  CalculateDistanceToField.h
  CalculateDistanceToFieldBoundary.h
  CollectFields.h
  ConvertFieldBasis.h
  ConvertFieldDataType.h
  FairMesh.h
  ProjectPointsOntoMesh.h
  RegisterWithCorrespondences.h
  ResampleRegularMesh.h
  ScaleFieldMeshAndData.h
  SetFieldOrMeshStringProperty.h
  CalculateFieldData5.h
  CalculateGradients.h
  CalculateVectorMagnitudes.h
  CalculateSignedDistanceToField.h
  ClipFieldByFunction3.h
  ClipFieldByMesh.h
  ConvertQuadSurfToTriSurf.h
  AlignMeshBoundingBoxes.h
  SetFieldNodes.h
  SetFieldData.h
  GetFieldData.h
  GetDomainBoundary.h
  GetFieldNodes.h
  JoinFields.h
  SetFieldDataToConstantValue.h
  SplitFieldByDomain.h
  SplitFieldByConnectedRegion.h
  CreateFieldData.h
  SplitFieldByDomain.h
  ApplyMappingMatrix.h
  MapFieldDataFromElemToNode.h
  MapFieldDataFromNodeToElem.h
  MapFieldDataOntoElems.h
  MapFieldDataOntoNodes.h
  MapFieldDataFromSourceToDestination.h
  FlipSurfaceNormals.h
  SwapFieldDataWithMatrixEntries.h
  ConvertIndicesToFieldData.h
  ConvertHexVolToTetVol.h
  ExtractSimpleIsosurface.h
  GetSliceFromStructuredFieldByIndices.h
  GenerateSinglePointProbeFromField.h
  GeneratePointSamplesFromField.h
  GeneratePointSamplesFromFieldOrWidget.h
  TransformMeshWithTransform.h
)

SET(Modules_Legacy_Fields_SRCS
  FlipSurfaceNormals.cc
  ClipVolumeByIsovalue.cc
  #GenerateElectrode.cc
  #PadRegularMesh.cc
  ResampleRegularMesh.cc
  #CalculateMeshConnector.cc
  #CalculateMeshCenter.cc
  #GetDomainStructure.cc
  RegisterWithCorrespondences.cc
  #SmoothVecFieldMedian.cc
  SetFieldDataToConstantValue.cc
  #RemoveUnusedNodes.cc
  MapFieldDataOntoNodes.cc
  MapFieldDataOntoElems.cc
  #CleanupTetMesh.cc
  #MatchDomainLabels.cc
  SplitFieldByDomain.cc
  AlignMeshBoundingBoxes.cc
  ProjectPointsOntoMesh.cc
  #GetMeshQualityField.cc
  #ApplyFilterToFieldData.cc
  ApplyMappingMatrix.cc
  #CalculateNodeNormals.cc
  BuildMappingMatrix.cc
  BuildMatrixOfSurfaceNormals.cc
  #GetCentroidsFromMesh.cc
  #ConvertMeshCoordinateSystem.cc
  ConvertFieldBasis.cc
  ConvertFieldDataType.cc
  #ChooseField.cc
  ClipFieldByFunction3.cc
  ClipFieldByMesh.cc
  #ClipFieldToFieldOrWidget.cc
  #ClipLatVolByIndicesOrWidget.cc
  #ConvertMatricesToMesh.cc
  #CollectPointClouds.cc
  #CoregisterPointClouds.cc
  #EditMeshBoundingBox.cc
  #ExtractIsosurfaceByFunction.cc
  #GenerateMedialAxisPoints.cc
  MapFieldDataFromSourceToDestination.cc
  #MaskLatVolWithTriSurf.cc
  #RemoveHexVolSheet.cc
  FairMesh.cc
  #GetAllSegmentationBoundaries.cc
  GetFieldBoundary.cc
  #ReportFieldGeometryMeasures.cc
  #SetFieldProperty.cc
  GetSliceFromStructuredFieldByIndices.cc
  #SubsampleStructuredFieldByIndices.cc
  JoinFields.cc
  CalculateGradients.cc
  ConvertHexVolToTetVol.cc
  #MergeFields.cc
  #InsertHexVolSheetAlongSurface.cc
  #MergeTriSurfs.cc
  #ClipVolumeByIsovalue.cc
  RefineMesh.cc
  SwapFieldDataWithMatrixEntries.cc
  #SwapNodeLocationsWithMatrixEntries.cc
  #ConvertLatVolDataFromElemToNode.cc
  #ConvertLatVolDataFromNodeToElem.cc
  #CalculateLatVolGradientsAtNodes.cc
  ConvertQuadSurfToTriSurf.cc
  #TransformPlanarMesh.cc
  #BuildPointCloudToLatVolMappingMatrix.cc
  GenerateSinglePointProbeFromField.cc
  GeneratePointSamplesFromFieldOrWidget.cc
  CreateLatVol.cc
  #CreateImage.cc
  #CreateStructHex.cc
  #ReportScalarFieldStats.cc
  GeneratePointSamplesFromField.cc
  #SelectFieldROIWithBoxWidget.cc
  SetFieldOrMeshStringProperty.cc
  #ConvertMeshToPointCloud.cc
  #ConvertMeshToIrregularMesh.cc
  #ConvertMeshToUnstructuredMesh.cc
  CalculateFieldData5.cc
  #CalculateFieldDataMetric.cc
  #CalculateMeshNodes.cc
  #QueryFieldData.cc
  TransformMeshWithTransform.cc
  CalculateVectorMagnitudes.cc
  #GenerateLinearSegments.cc
  CalculateDistanceToField.cc
  CalculateDistanceToFieldBoundary.cc
  CalculateSignedDistanceToField.cc
  #CalculateIsInsideField.cc
  #CalculateInsideWhichField.cc
  ConvertIndicesToFieldData.cc
  CreateFieldData.cc
  GetFieldData.cc
  SetFieldData.cc
  GetFieldNodes.cc
  SetFieldNodes.cc
  #MapFieldDataOntoNodesRadialbasis.cc
  #SelectAndSetFieldData.cc
  #SelectAndSetFieldData3.cc
  #SplitNodesByDomain.cc
  SplitFieldByConnectedRegion.cc
  GetDomainBoundary.cc
  CollectFields.cc
  ScaleFieldMeshAndData.cc
  #CalculateProfile.cc
  MapFieldDataFromElemToNode.cc
  MapFieldDataFromNodeToElem.cc
  ExtractSimpleIsosurface.cc
)

IF(WITH_TETGEN)
  SET(Modules_Legacy_Fields_SRCS ${Modules_Legacy_Fields_SRCS}
    InterfaceWithTetGen.cc
    InterfaceWithTetGenImpl.cc
  )
  SET(Modules_Legacy_Fields_HEADERS ${Modules_Legacy_Fields_HEADERS}
    InterfaceWithTetGen.h
    InterfaceWithTetGenImpl.h
  )
ENDIF()

SCIRUN_ADD_LIBRARY(Modules_Legacy_Fields
  ${Modules_Legacy_Fields_HEADERS}
  ${Modules_Legacy_Fields_SRCS}
)

TARGET_LINK_LIBRARIES(Modules_Legacy_Fields
  Dataflow_Network
  Core_Algorithms_Legacy_Fields
  Algorithms_Base
  Core_Basis
  Core_Datatypes
  Core_Datatypes_Legacy_Bundle
  Core_Exceptions_Legacy
  Core_Geometry_Primitives
  Core_Math
  Core_Parser
  Core_Util_Legacy
  Graphics_Glyphs
  Graphics_Datatypes
)

IF(WITH_TETGEN)
  TARGET_LINK_LIBRARIES(Modules_Legacy_Fields ${TETGEN_LIBRARY})
ENDIF()

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Legacy_Fields)
ENDIF()
