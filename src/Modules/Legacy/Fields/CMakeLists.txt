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
  ConvertMatricesToMesh.h
  ConvertQuadSurfToTriSurf.h
  ConvertMeshToPointCloud.h
  ConvertMeshToUnstructuredMesh.h
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
  ApplyFilterToFieldData.h
  MapFieldDataFromElemToNode.h
  MapFieldDataFromNodeToElem.h
  MapFieldDataOntoElems.h
  MapFieldDataOntoNodes.h
  MapFieldDataFromSourceToDestination.h
  MapFieldDataOntoNodesRadialbasis.h
  ReportFieldGeometryMeasures.h
  ReportScalarFieldStats.h
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
  GetMeshQualityField.h
  RemoveUnusedNodes.h
  CleanupTetMesh.h
  CalculateInsideWhichField.h
  ReorderNormalCoherently.h
  CalculateMeshCenter.h
  CreateImage.h
  GetCentroidsFromMesh.h
  SmoothVecFieldMedian.h
  CalculateMeshNodes.h
  GenerateElectrode.h
  InterfaceWithTetGen.h
  GenerateNodeNormals.h
  CalculateFieldDataMetric.h
  BuildElemLeadField.h
  MergeTriSurfs.h
)

SET(Modules_Legacy_Fields_SRCS
  FlipSurfaceNormals.cc
  ClipVolumeByIsovalue.cc
  GenerateElectrode.cc
  #PadRegularMesh.cc
  ResampleRegularMesh.cc
  #CalculateMeshConnector.cc
  CalculateMeshCenter.cc
  #GetDomainStructure.cc
  RegisterWithCorrespondences.cc
  SmoothVecFieldMedian.cc
  SetFieldDataToConstantValue.cc
  RemoveUnusedNodes.cc
  MapFieldDataOntoNodes.cc
  MapFieldDataOntoElems.cc
  CleanupTetMesh.cc
  #MatchDomainLabels.cc
  SplitFieldByDomain.cc
  AlignMeshBoundingBoxes.cc
  ProjectPointsOntoMesh.cc
  GetMeshQualityField.cc
  ApplyFilterToFieldData.cc
  ApplyMappingMatrix.cc
  #CalculateNodeNormals.cc
  BuildMappingMatrix.cc
  BuildMatrixOfSurfaceNormals.cc
  GetCentroidsFromMesh.cc
  #ConvertMeshCoordinateSystem.cc
  ConvertFieldBasis.cc
  ConvertFieldDataType.cc
  #ChooseField.cc
  ClipFieldByFunction3.cc
  ClipFieldByMesh.cc
  #ClipFieldToFieldOrWidget.cc
  #ClipLatVolByIndicesOrWidget.cc
  ConvertMatricesToMesh.cc
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
  ReportFieldGeometryMeasures.cc
  #SetFieldProperty.cc
  GetSliceFromStructuredFieldByIndices.cc
  #SubsampleStructuredFieldByIndices.cc
  JoinFields.cc
  CalculateGradients.cc
  ConvertHexVolToTetVol.cc
  #MergeFields.cc
  #InsertHexVolSheetAlongSurface.cc
  MergeTriSurfs.cc
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
  CreateImage.cc
  #CreateStructHex.cc
  ReportScalarFieldStats.cc
  GeneratePointSamplesFromField.cc
  #SelectFieldROIWithBoxWidget.cc
  SetFieldOrMeshStringProperty.cc
  ConvertMeshToPointCloud.cc
  #ConvertMeshToIrregularMesh.cc
  ConvertMeshToUnstructuredMesh.cc
  CalculateFieldData5.cc
  CalculateFieldDataMetric.cc
  CalculateMeshNodes.cc
  #QueryFieldData.cc
  TransformMeshWithTransform.cc
  CalculateVectorMagnitudes.cc
  #GenerateLinearSegments.cc
  CalculateDistanceToField.cc
  CalculateDistanceToFieldBoundary.cc
  CalculateSignedDistanceToField.cc
  #CalculateIsInsideField.cc
  CalculateInsideWhichField.cc
  ConvertIndicesToFieldData.cc
  CreateFieldData.cc
  GetFieldData.cc
  SetFieldData.cc
  GetFieldNodes.cc
  SetFieldNodes.cc
  MapFieldDataOntoNodesRadialbasis.cc
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
  ReorderNormalCoherently.cc
  InterfaceWithTetGen.cc
  GenerateNodeNormals.cc
  #CompositeModuleTestGFB_FM.cc
  BuildElemLeadField.cc
)

IF(WITH_TETGEN)
  SET(Modules_Legacy_Fields_SRCS ${Modules_Legacy_Fields_SRCS}
    InterfaceWithTetGenImpl.cc
  )
  SET(Modules_Legacy_Fields_HEADERS ${Modules_Legacy_Fields_HEADERS}
    InterfaceWithTetGenImpl.h
  )
  ADD_DEFINITIONS(-DWITH_TETGEN)
ENDIF()

# Create the target (unchanged)
SCIRUN_ADD_LIBRARY(Modules_Legacy_Fields
  ${Modules_Legacy_Fields_HEADERS}
  ${Modules_Legacy_Fields_SRCS}
)

# ---- Convert to keyword signature and keep all links here ----
target_link_libraries(Modules_Legacy_Fields
  PRIVATE
    Dataflow_Network
    # TODO: remove
    Core_Serialization_Network
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
    Graphics_Widgets
    Graphics_Datatypes
)

# ---- TetGen wiring (include + link + define) ----
if(WITH_TETGEN)
  message(STATUS "[Diag/Tetgen/Fields] WITH_TETGEN='${WITH_TETGEN}'")
  message(STATUS "[Diag/Tetgen/Fields] TETGEN_INCLUDE_DIR='${TETGEN_INCLUDE_DIR}'")
  message(STATUS "[Diag/Tetgen/Fields] TETGEN_LIB_DIR='${TETGEN_LIB_DIR}'")
  message(STATUS "[Diag/Tetgen/Fields] TETGEN_LIBRARY='${TETGEN_LIBRARY}'")
  if(TARGET Tetgen::Tetgen)
    message(STATUS "[Diag/Tetgen/Fields] TARGET Tetgen::Tetgen is visible here")
  endif()
  # Add the include directory so 'tetgen.h' is found
  if(DEFINED TETGEN_INCLUDE_DIR AND EXISTS "${TETGEN_INCLUDE_DIR}")
    target_include_directories(Modules_Legacy_Fields PRIVATE "${TETGEN_INCLUDE_DIR}")
  else()
    message(FATAL_ERROR "WITH_TETGEN=ON but TETGEN_INCLUDE_DIR is not set or does not exist.")
  endif()

  # Link the TetGen library passed from superbuild
  if(DEFINED TETGEN_LIBRARY AND EXISTS "${TETGEN_LIBRARY}")
    target_link_libraries(Modules_Legacy_Fields PRIVATE "${TETGEN_LIBRARY}")
  else()
    message(FATAL_ERROR "WITH_TETGEN=ON but TETGEN_LIBRARY is not set or does not exist.")
  endif()

  # Define TETLIBRARY when consuming TetGen as a library
  target_compile_definitions(Modules_Legacy_Fields PRIVATE TETLIBRARY)
endif()

# Shared library define (unchanged)
if(BUILD_SHARED_LIBS)
  add_definitions(-DBUILD_Modules_Legacy_Fields)
endif()