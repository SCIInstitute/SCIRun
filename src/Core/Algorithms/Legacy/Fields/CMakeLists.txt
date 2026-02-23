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


SET(Core_Algorithms_Legacy_Fields_HEADERS
  ClipMesh/ClipMeshBySelection.h
  ClipMesh/ClipMeshByIsovalue.h
  ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.h
  ConvertMeshType/ConvertMeshToUnstructuredMesh.h
  DistanceField/CalculateSignedDistanceField.h
  DistanceField/CalculateDistanceField.h
  Mapping/ApplyMappingMatrix.h
  FieldData/BuildMatrixOfSurfaceNormalsAlgo.h
  #Mapping/ApplyMappingMatrix.h
  FieldData/CalculateFieldDataMetric.h
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
  FieldData/SmoothVecFieldMedianAlgo.h
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
  FilterFieldData/DilateFieldData.h
  FilterFieldData/ErodeFieldData.h
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
  MeshData/GetMeshQualityFieldAlgo.h
  Cleanup/RemoveUnusedNodes.h
  Cleanup/CleanupTetMesh.h
  DistanceField/CalculateInsideWhichFieldAlgorithm.h
  Cleanup/ReorderNormalCoherentlyAlgo.h
  MeshDerivatives/CalculateMeshCenterAlgo.h
  MeshDerivatives/GetCentroids.h
  MeshDerivatives/CalculateBundleDifference.h
  )

SET(Core_Algorithms_Legacy_Fields_SRCS
  RegisterWithCorrespondences.cc
  MeshData/FlipSurfaceNormals.cc
  Cleanup/RemoveUnusedNodes.cc
  Cleanup/CleanupTetMesh.cc
  #ClipMesh/ClipMeshByIsovalue.cc
  ClipMesh/ClipMeshBySelection.cc
  ClipMesh/ClipMeshByIsovalue.cc
  #CollectFields/CollectPointClouds.cc
  ConvertMeshType/ConvertMeshToPointCloudMeshAlgo.cc
  ConvertMeshType/ConvertMeshToTetVolMesh.cc
  ConvertMeshType/ConvertMeshToTriSurfMeshAlgo.cc
  ConvertMeshType/ConvertMeshToUnstructuredMesh.cc
  ConvertMeshType/ConvertMeshToIrregularMesh.cc
  #ConvertMeshType/ConvertLatVolDataFromElemToNode.cc
  #ConvertMeshType/ConvertLatVolDataFromNodeToElem.cc
  #CompareFields/CompareFields.cc
  #CompareFields/SimilarMeshes.cc
  #CreateMesh/CreateMeshFromNrrd.cc

  DistanceField/CalculateDistanceField.cc
  DistanceField/CalculateIsInsideField.cc
  DistanceField/CalculateInsideWhichFieldAlgorithm.cc
  DistanceField/CalculateSignedDistanceField.cc
  DomainFields/GetDomainBoundaryAlgo.cc
  #DomainFields/GetDomainStructure.cc
  #DomainFields/MatchDomainLabels.cc
  #DomainFields/SplitNodesByDomain.cc
  DomainFields/SplitFieldByDomainAlgo.cc
  FieldData/CalculateGradientsAlgo.cc
  #FieldData/CalculateLatVolGradientsAtNodes.cc
  FieldData/CalculateVectorMagnitudesAlgo.cc
  FieldData/CalculateFieldDataMetric.cc
  FieldData/ConvertFieldDataType.cc
  FieldData/ConvertFieldBasisType.cc
  FieldData/SwapFieldDataWithMatrixEntriesAlgo.cc
  FieldData/ConvertIndicesToFieldDataAlgo.cc
  #FieldData/GetFieldData.cc
  #FieldData/ConvertMappingMatrixToFieldData.cc
  FieldData/SetFieldData.cc
  FieldData/SetFieldDataToConstantValue.cc
  FieldData/SmoothVecFieldMedianAlgo.cc
  FilterFieldData/DilateFieldData.cc
  FilterFieldData/ErodeFieldData.cc
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
  MeshData/GetMeshQualityFieldAlgo.cc
  #MeshDerivatives/CalculateMeshConnector.cc
  MeshDerivatives/CalculateMeshCenterAlgo.cc
  MeshDerivatives/GetCentroids.cc
  MeshDerivatives/GetFieldBoundaryAlgo.cc
  #MeshDerivatives/GetBoundingBox.cc
  MeshDerivatives/SplitByConnectedRegion.cc
  MeshDerivatives/ExtractSimpleIsosurfaceAlgo.cc
  MeshDerivatives/CalculateBundleDifference.cc
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
  Cleanup/ReorderNormalCoherentlyAlgo.cc
)

SCIRUN_ADD_LIBRARY(Core_Algorithms_Legacy_Fields
  ${Core_Algorithms_Legacy_Fields_HEADERS}
  ${Core_Algorithms_Legacy_Fields_SRCS}
)

# ==========================================================
# Teem discovery (prefer config, fallback to superbuild vars)
# Static consumption: ensure headers do NOT use dllimport.
# ==========================================================
# Expect these from superbuild:
#   Teem_INCLUDE_DIR -> <.../Externals/Install/Teem_external>/include
#   Teem_LIB_DIR     -> <.../Externals/Install/Teem_external>/lib

# Prefer a config package if available (uncommon for Teem, but harmless):
find_package(Teem CONFIG QUIET)

if(TARGET Teem::teem)
  # If this comes from a config that exported a SHARED target,
  # force static-consumer macros at call sites:
  set_property(TARGET Teem::teem APPEND PROPERTY
    INTERFACE_COMPILE_DEFINITIONS "TEEM_STATIC"
  )
  # If your Teem was built with zlib and the config exposes it, link transitively.
  # Otherwise we’ll add Zlib explicitly below when we link this target.
  set(_teem_target Teem::teem)
else()
  if(NOT Teem_INCLUDE_DIR OR NOT Teem_LIB_DIR)
    message(FATAL_ERROR
      "Teem not configured: need Teem_INCLUDE_DIR and Teem_LIB_DIR from the superbuild.\n"
      "Teem_INCLUDE_DIR='${Teem_INCLUDE_DIR}'  Teem_LIB_DIR='${Teem_LIB_DIR}'")
  endif()

  # Resolve static library for each config
  set(_TEEM_LIB_RELEASE "")
  set(_TEEM_LIB_DEBUG   "")

  if(WIN32)
    # Common names used by Teem on Windows
    if(EXISTS "${Teem_LIB_DIR}/teem.lib")
      set(_TEEM_LIB_RELEASE "${Teem_LIB_DIR}/teem.lib")
    elseif(EXISTS "${Teem_LIB_DIR}/libteem.lib")
      set(_TEEM_LIB_RELEASE "${Teem_LIB_DIR}/libteem.lib")
    endif()

    # Debug may use the same name or have a trailing 'd'
    if(EXISTS "${Teem_LIB_DIR}/teemd.lib")
      set(_TEEM_LIB_DEBUG "${Teem_LIB_DIR}/teemd.lib")
    elseif(EXISTS "${Teem_LIB_DIR}/teem.lib")
      set(_TEEM_LIB_DEBUG "${Teem_LIB_DIR}/teem.lib")
    elseif(EXISTS "${Teem_LIB_DIR}/libteemd.lib")
      set(_TEEM_LIB_DEBUG "${Teem_LIB_DIR}/libteemd.lib")
    elseif(EXISTS "${Teem_LIB_DIR}/libteem.lib")
      set(_TEEM_LIB_DEBUG "${Teem_LIB_DIR}/libteem.lib")
    endif()
  elseif(APPLE)
    # Prefer static if available
    if(EXISTS "${Teem_LIB_DIR}/libteem.a")
      set(_TEEM_LIB_RELEASE "${Teem_LIB_DIR}/libteem.a")
      set(_TEEM_LIB_DEBUG   "${Teem_LIB_DIR}/libteem.a")
    elseif(EXISTS "${Teem_LIB_DIR}/libteem.dylib")
      # Shared exists but we still treat consumer as static (headers need static defines)
      set(_TEEM_LIB_RELEASE "${Teem_LIB_DIR}/libteem.dylib")
      set(_TEEM_LIB_DEBUG   "${Teem_LIB_DIR}/libteem.dylib")
    endif()
  else()
    if(EXISTS "${Teem_LIB_DIR}/libteem.a")
      set(_TEEM_LIB_RELEASE "${Teem_LIB_DIR}/libteem.a")
      set(_TEEM_LIB_DEBUG   "${Teem_LIB_DIR}/libteem.a")
    elseif(EXISTS "${Teem_LIB_DIR}/libteem.so")
      set(_TEEM_LIB_RELEASE "${Teem_LIB_DIR}/libteem.so")
      set(_TEEM_LIB_DEBUG   "${Teem_LIB_DIR}/libteem.so")
    endif()
  endif()

  # Last-resort search
  if(NOT _TEEM_LIB_RELEASE)
    find_library(TEEM_LIBRARY_RELEASE
      NAMES teem libteem
      HINTS "${Teem_LIB_DIR}" "${CMAKE_PREFIX_PATH}" "${CMAKE_PREFIX_PATH}/lib" "${CMAKE_PREFIX_PATH}/lib64"
    )
    if(TEEM_LIBRARY_RELEASE)
      set(_TEEM_LIB_RELEASE "${TEEM_LIBRARY_RELEASE}")
      set(_TEEM_LIB_DEBUG   "${TEEM_LIBRARY_RELEASE}")
    endif()
  endif()

  if(NOT _TEEM_LIB_RELEASE OR NOT EXISTS "${_TEEM_LIB_RELEASE}")
    message(FATAL_ERROR
      "Teem library not found. Looked under '${Teem_LIB_DIR}'.\n"
      "Teem_INCLUDE_DIR='${Teem_INCLUDE_DIR}'  Teem_LIB_DIR='${Teem_LIB_DIR}'\n"
      "Ensure the Teem external installs a static lib (teem.lib/libteem.a).")
  endif()

  # Create imported STATIC target and propagate include paths and *static* defines
  add_library(Teem::teem STATIC IMPORTED GLOBAL)
  set_target_properties(Teem::teem PROPERTIES
    IMPORTED_LOCATION_RELEASE         "${_TEEM_LIB_RELEASE}"
    IMPORTED_LOCATION_DEBUG           "${_TEEM_LIB_DEBUG}"
    INTERFACE_INCLUDE_DIRECTORIES     "${Teem_INCLUDE_DIR}"
    # This disables __declspec(dllimport) in Teem headers for static consumption.
    # If you still see __imp_* after a clean rebuild, replace with TEEM_STATIC_DEFINE
    # or add module-level defines like AIR_STATIC;NRRD_STATIC, depending on your Teem version.
    INTERFACE_COMPILE_DEFINITIONS     "TEEM_STATIC"
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
  )

  set(_teem_target Teem::teem)
endif()

# ---- Link this library ----
# Convert your legacy link line to target-based + PRIVATE for clarity.
target_link_libraries(Core_Algorithms_Legacy_Fields
  PRIVATE
    Core_Datatypes
    Core_Datatypes_Legacy_Field
    Core_Datatypes_Legacy_Nrrd
    Core_Datatypes_Legacy_Bundle
    Algorithms_Base
    Core_Exceptions_Legacy
    Core_Thread
    Core_Geometry_Primitives
    Core_Basis
    Algorithms_Math
    ${_teem_target}
)

# ==========================================================
# Zlib is REQUIRED because Teem's gzio uses it (deflate/inflate/crc32).
# Use modern imported target; fall back to variables from the superbuild.
# ==========================================================
set(_zlib_target "")

# Prefer an existing target if the top-level already created it.
if(TARGET ZLIB::ZLIB)
  set(_zlib_target ZLIB::ZLIB)
else()
  # Try module find (honors ZLIB_ROOT and CMAKE_PREFIX_PATH).
  find_package(ZLIB QUIET)

  if(TARGET ZLIB::ZLIB)
    set(_zlib_target ZLIB::ZLIB)
  else()
    # Fallbacks to superbuild-provided variables.
    # Newer CMake may expose ZLIB_LIBRARIES/ZLIB_INCLUDE_DIRS;
    # older uses ZLIB_LIBRARY/ZLIB_INCLUDE_DIR.
    if(ZLIB_LIBRARY AND ZLIB_INCLUDE_DIR)
      add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
      set_target_properties(ZLIB::ZLIB PROPERTIES
        IMPORTED_LOCATION "${ZLIB_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}"
      )
      set(_zlib_target ZLIB::ZLIB)
    elseif(ZLIB_LIBRARIES AND ZLIB_INCLUDE_DIRS)
      add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
      set_target_properties(ZLIB::ZLIB PROPERTIES
        IMPORTED_LOCATION "${ZLIB_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}"
      )
      set(_zlib_target ZLIB::ZLIB)
    else()
      message(FATAL_ERROR
        "Zlib not found. SCIRun needs it to link Teem (gzio).\n"
        "Provide ZLIB_ROOT or ZLIB_LIBRARY/ZLIB_INCLUDE_DIR from the superbuild.\n"
        "ZLIB_LIBRARY='${ZLIB_LIBRARY}'  ZLIB_LIBRARIES='${ZLIB_LIBRARIES}'\n"
        "ZLIB_INCLUDE_DIR='${ZLIB_INCLUDE_DIR}'  ZLIB_INCLUDE_DIRS='${ZLIB_INCLUDE_DIRS}'")
    endif()
  endif()
endif()

# Link zlib unconditionally (Teem uses it)
target_link_libraries(Core_Algorithms_Legacy_Fields PRIVATE ${_zlib_target})

# Export macro for this DLL, unchanged
if(BUILD_SHARED_LIBS)
  target_compile_definitions(Core_Algorithms_Legacy_Fields PRIVATE BUILD_Core_Algorithms_Legacy_Fields)
endif()