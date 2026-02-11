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


# ======================================
# Core/Datatypes - modern CMake (target-based)
# ======================================

# ---- Sources & headers ----
set(Core_Datatypes_SRCS
  BlockMatrix.cc
  Color.cc
  ColorMap.cc
  Datatype.cc
  Geometry.cc
  Material.cc
  Matrix.cc
  MatrixAlgorithms.cc
  MatrixTypeConversions.cc
  PropertyManagerExtensions.cc
  Scalar.cc
  SparseRowMatrixFromMap.cc
  String.cc
  MetadataObject.cc
  Feedback.cc
)

set(Core_Datatypes_HEADERS
  BlockMatrix.h
  Color.h
  ColorMap.h
  Datatype.h
  DatatypeFwd.h
  DenseMatrix.h
  TensorBase.h
  DyadicTensor.h
  Dyadic3DTensor.h
  TensorFwd.h
  DenseColumnMatrix.h
  Geometry.h
  HasId.h
  Material.h
  Matrix.h
  MatrixAlgorithms.h
  MatrixComparison.h
  MatrixFwd.h
  MatrixIO.h
  MatrixTypeConversions.h
  MatrixMathVisitors.h
  MetadataObject.h
  PropertyManagerExtensions.h
  Scalar.h
  share.h
  SparseRowMatrix.h
  SparseRowMatrixFromMap.h
  String.h
  Feedback.h
)

# ---- Library target ----
SCIRUN_ADD_LIBRARY(Core_Datatypes
  ${Core_Datatypes_HEADERS}
  ${Core_Datatypes_SRCS}
)

# Shared lib export define (kept behavior)
if(BUILD_SHARED_LIBS)
  target_compile_definitions(Core_Datatypes PRIVATE BUILD_Core_Datatypes)
endif()

# ---- Dependencies (existing) ----
target_link_libraries(Core_Datatypes
  PRIVATE
    Core_Persistent
    Core_Datatypes_Legacy_Base
    Core_Geometry_Primitives
)

# ==========================================================
# Eigen (header-only): consume superbuild-exported include dir
# ==========================================================

# Prefer superbuild's direct export
set(_eigen_inc "")
if(DEFINED Eigen_INCLUDE_DIR AND EXISTS "${Eigen_INCLUDE_DIR}/Eigen/Dense")
  set(_eigen_inc "${Eigen_INCLUDE_DIR}")
elseif(DEFINED SCIRUN_EIGEN_INCLUDE AND EXISTS "${SCIRUN_EIGEN_INCLUDE}/Eigen/Dense")
  set(_eigen_inc "${SCIRUN_EIGEN_INCLUDE}")
endif()

# Final check
if(NOT _eigen_inc)
  message(FATAL_ERROR
    "Eigen headers not found. Provide one of:\n"
    "  -DEigen_INCLUDE_DIR=E:/PROJECTS/SCIRun/build/Externals/Install/Eigen_external/include\n"
    "  -DSCIRUN_EIGEN_INCLUDE=E:/PROJECTS/SCIRun/build/Externals/Install/Eigen_external/include\n"
    "Or adjust to match your superbuild output.")
endif()

# Create a simple header-only target that mirrors Eigen3::Eigen semantics
if(NOT TARGET Eigen3::Eigen)
  add_library(Eigen3::Eigen INTERFACE IMPORTED)
  set_target_properties(Eigen3::Eigen PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${_eigen_inc}"
  )
endif()

# Attach to this target (and any others that include <Eigen/...>)
target_link_libraries(Core_Datatypes PRIVATE Eigen3::Eigen)

# ===========================
# Tests and subdirectories
# ===========================
SCIRUN_ADD_TEST_DIR(Tests)
add_subdirectory(Mesh)
add_subdirectory(Legacy)