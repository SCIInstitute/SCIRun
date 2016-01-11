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

SET(Core_Algorithms_Legacy_Fields_HEADERS
  ClipMesh/ClipMeshBySelection.h
  ClipMesh/ClipMeshByIsovalue.h
  ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.h
  DistanceField/CalculateSignedDistanceField.h
  DistanceField/CalculateDistanceField.h
  Mapping/ApplyMappingMatrix.h
  FieldData/BuildMatrixOfSurfaceNormalsAlgo.h
  #Mapping/ApplyMappingMatrix.h
  #FieldData/CalculateFieldDataMetric.h
  FieldData/ConvertIndicesToFieldDataAlgo.h
  FieldData/CalculateGradientsAlgo.h
  FieldData/CalculateVectorMagnitudesAlgo.h
  #FieldData/ConvertMappingMatrixToFieldData.h
  FieldData/ConvertFieldDataType.h
  #FieldData/CalculateLatVolGradientsAtNodes.h
  #FieldData/GetFieldData.h
  FieldData/SetFieldData.h
  FieldData/SetFieldDataToConstantValue.h
  FieldData/SwapFieldDataWithMatrixEntriesAlgo.h
  #FieldData/SmoothVecFieldMedian.h
  Mapping/BuildMappingMatrixAlgo.h
  DomainFields/GetDomainBoundaryAlgo.h
  MeshDerivatives/GetFieldBoundaryAlgo.h
  MeshDerivatives/SplitByConnectedRegion.h
  MeshDerivatives/ExtractSimpleIsosurfaceAlgo.h
  ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.h
  ConvertMeshType/ConvertMeshToIrregularMesh.h
  ConvertMeshType/ConvertMeshToTetVolMesh.h
  TransformMesh/AlignMeshBoundingBoxes.h
  MeshData/SetMeshNodes.h
  MeshData/GetMeshNodes.h
  MergeFields/JoinFieldsAlgo.h
  MergeFields/AppendFieldsAlgo.h
  DomainFields/SplitFieldByDomainAlgo.h
  #MeshData/GetMeshData.h
  FieldData/GetFieldData.h
  Mapping/MapFieldDataFromElemToNode.h
  Mapping/MapFieldDataFromNodeToElem.h
  Mapping/MapFieldDataOntoNodes.h
  Mapping/MapFieldDataOntoElems.h
  Mapping/MappingDataSource.h
  Mapping/MapFieldDataFromSourceToDestination.h
  ResampleMesh/ResampleRegularMesh.h
  SmoothMesh/FairMesh.h
  FieldData/ConvertFieldBasisType.h
  TransformMesh/ScaleFieldMeshAndData.h
  TransformMesh/ProjectPointsOntoMesh.h
  TransformMesh/TransformMeshWithTransform.h
  MeshData/FlipSurfaceNormals.h
  RefineMesh/RefineMesh.h
  MarchingCubes/BaseMC.h
  MarchingCubes/HexMC.h
  MarchingCubes/UHexMC.h
  MarchingCubes/TetMC.h
  MarchingCubes/TriMC.h
  MarchingCubes/MarchingCubes.h
  MarchingCubes/QuadMC.h
  MarchingCubes/EdgeMC.h
  MarchingCubes/PrismMC.h
  MarchingCubes/mcube2.h
  RefineMesh/RefineMeshCurveAlgoV.h
  RefineMesh/RefineMeshHexVolAlgoV.h
  RefineMesh/RefineMeshQuadSurfAlgoV.h
  RefineMesh/RefineMeshTetVolAlgoV.h
  RefineMesh/RefineMeshTriSurfAlgoV.h
  RefineMesh/EdgePairHash.h
  StreamLines/StreamLineIntegrators.h
  StreamLines/GenerateStreamLines.h
  RegisterWithCorrespondences.h
  SampleField/GeneratePointSamplesFromField.h
  DistanceField/CalculateIsInsideField.h
)

SET(Core_Algorithms_Legacy_Fields_SRCS
  RegisterWithCorrespondences.cc
  MeshData/FlipSurfaceNormals.cc
  #Cleanup/CleanupTetMesh.cc
  #Cleanup/RemoveUnusedNodes.cc
  #ClipMesh/ClipMeshByIsovalue.cc
  ClipMesh/ClipMeshBySelection.cc
  ClipMesh/ClipMeshByIsovalue.cc
  #CollectFields/CollectPointClouds.cc
  ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.cc
  ConvertMeshType/ConvertMeshToTetVolMesh.cc
  ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.cc
  #ConvertMeshType/ConvertMeshToUnstructuredMesh.cc
  ConvertMeshType/ConvertMeshToIrregularMesh.cc
  #ConvertMeshType/ConvertLatVolDataFromElemToNode.cc
  #ConvertMeshType/ConvertLatVolDataFromNodeToElem.cc
  #CompareFields/CompareFields.cc
  #CompareFields/SimilarMeshes.cc
  #CreateMesh/CreateMeshFromNrrd.cc

  DistanceField/CalculateDistanceField.cc
  DistanceField/CalculateIsInsideField.cc
  #DistanceField/CalculateInsideWhichField.cc
  DistanceField/CalculateSignedDistanceField.cc
  DomainFields/GetDomainBoundaryAlgo.cc
  #DomainFields/GetDomainStructure.cc
  #DomainFields/MatchDomainLabels.cc
  #DomainFields/SplitNodesByDomain.cc
  DomainFields/SplitFieldByDomainAlgo.cc
  FieldData/CalculateGradientsAlgo.cc
  #FieldData/CalculateLatVolGradientsAtNodes.cc
  FieldData/CalculateVectorMagnitudesAlgo.cc
  #FieldData/CalculateFieldDataMetric.cc
  FieldData/ConvertFieldDataType.cc
  FieldData/ConvertFieldBasisType.cc
  FieldData/SwapFieldDataWithMatrixEntriesAlgo.cc
  FieldData/ConvertIndicesToFieldDataAlgo.cc
  #FieldData/GetFieldData.cc
  #FieldData/ConvertMappingMatrixToFieldData.cc
  FieldData/SetFieldData.cc
  FieldData/SetFieldDataToConstantValue.cc
  #FieldData/SmoothVecFieldMedian.cc
  #FilterFieldData/DilateFieldData.cc
  #FilterFieldData/ErodeFieldData.cc
  #FilterFieldData/TriSurfPhaseFilter.cc
  #FindNodes/FindClosestNode.cc
  #FindNodes/FindClosestNodeByValue.cc
  FieldData/BuildMatrixOfSurfaceNormalsAlgo.cc
  Mapping/BuildMappingMatrixAlgo.cc
  Mapping/MapFieldDataFromElemToNode.cc
  Mapping/MapFieldDataFromNodeToElem.cc
  Mapping/MapFieldDataFromSourceToDestination.cc
  Mapping/MappingDataSource.cc
  Mapping/MapFieldDataOntoNodes.cc
  Mapping/MapFieldDataOntoElems.cc
  #Mapping/MapFromPointField.cc
  #Mapping/FindClosestNodesFromPointField.cc
  MarchingCubes/BaseMC.cc
  MarchingCubes/TetMC.h
  MarchingCubes/EdgeMC.cc
  MarchingCubes/HexMC.cc
  MarchingCubes/MarchingCubes.cc
  MarchingCubes/mcube2.cc
  MarchingCubes/PrismMC.cc
  MarchingCubes/QuadMC.cc
  MarchingCubes/TetMC.cc
  MarchingCubes/TriMC.cc
  MarchingCubes/UHexMC.cc
  #MedialAxis/MedialAxis.cc
  #MedialAxis/MedialAxis2.cc
  MergeFields/AppendFieldsAlgo.cc
  MergeFields/JoinFieldsAlgo.cc
  MeshData/GetMeshNodes.cc
  MeshData/SetMeshNodes.cc
  #MeshData/GetMeshData.cc
  FieldData/GetFieldData.cc
  Mapping/ApplyMappingMatrix.cc
  #MeshData/GetSurfaceNodeNormals.cc
  #MeshData/GetSurfaceElemNormals.cc
  #MeshData/GetMeshQualityField.cc
  #MeshDerivatives/CalculateMeshConnector.cc
  #MeshDerivatives/CalculateMeshCenter.cc
  #MeshDerivatives/GetCentroids.cc
  MeshDerivatives/GetFieldBoundaryAlgo.cc
  #MeshDerivatives/GetBoundingBox.cc
  MeshDerivatives/SplitByConnectedRegion.cc
  MeshDerivatives/ExtractSimpleIsosurfaceAlgo.cc
  RefineMesh/RefineMesh.cc
  RefineMesh/RefineMeshCurveAlgoV.cc
  RefineMesh/RefineMeshHexVolAlgoV.cc
  RefineMesh/RefineMeshQuadSurfAlgoV.cc
  RefineMesh/RefineMeshTetVolAlgoV.cc
  RefineMesh/RefineMeshTriSurfAlgoV.cc
  ResampleMesh/ResampleRegularMesh.cc
  #ResampleMesh/PadRegularMesh.cc
  SampleField/GeneratePointSamplesFromField.cc
  SmoothMesh/FairMesh.cc
  StreamLines/StreamLineIntegrators.cc
  StreamLines/GenerateStreamLines.cc
  TransformMesh/AlignMeshBoundingBoxes.cc
  TransformMesh/TransformMeshWithTransform.cc
  #TransformMesh/ConvertMeshCoordinateSystem.cc
  TransformMesh/ScaleFieldMeshAndData.cc
  #TransformMesh/TransformPlanarMesh.cc
  #TransformMesh/GeneratePolarProjection.cc
  TransformMesh/ProjectPointsOntoMesh.cc
  #TransformMesh/PolarProjectionTools.cc
  #TransformMesh/GeneratePolarProjectionImage.cc
  #TracePoints/TracePoints.cc
)

SCIRUN_ADD_LIBRARY(Core_Algorithms_Legacy_Fields
  ${Core_Algorithms_Legacy_Fields_HEADERS}
  ${Core_Algorithms_Legacy_Fields_SRCS}
)

TARGET_LINK_LIBRARIES(Core_Algorithms_Legacy_Fields
  Core_Datatypes
  Core_Datatypes_Legacy_Field
  Core_Datatypes_Legacy_Nrrd
  Algorithms_Base
  Core_Exceptions_Legacy
  Core_Thread
  Core_Geometry_Primitives
  Core_Basis
  Algorithms_Math
)

IF(FALSE)
  IF(WITH_TETGEN)
    TARGET_LINK_LIBRARIES(Core_Algorithms_Legacy_Fields
      ${TETGEN_LIBRARY}
    )
  ENDIF()
ENDIF()

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Algorithms_Legacy_Fields)
ENDIF(BUILD_SHARED_LIBS)
