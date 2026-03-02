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


SET(Modules_Math_SRCS
  #ConvertMatrixType.cc
  #SelectSubMatrix.cc
  #GetSubmatrix.cc
  AppendMatrix.cc
  CreateGeometricTransform.cc
  ConvertMatrixToScalar.cc
  ConvertScalarToMatrix.cc
  CreateMatrix.cc
  CreateComplexMatrix.cc
  EvaluateLinearAlgebraUnary.cc
  EvaluateLinearAlgebraBinary.cc
  GetMatrixSlice.cc
  ReportMatrixInfo.cc
  ReportComplexMatrixInfo.cc
  ReportMatrixSliceMeasure.cc
  SolveLinearSystem.cc
  SolveComplexLinearSystem.cc
  ComputePCA.cc
  ConvertRealToComplexMatrix.cc
  ConvertComplexToRealMatrix.cc
  BooleanCompare.cc
  DisplayHistogram.cc
  BasicPlotter.cc
  AdvancedPlotter.cc
  ResizeMatrix.cc
  CreateStandardMatrix.cc
  ComputeTensorUncertainty.cc
)

SET(Modules_Math_HEADERS
  #ConvertMatrixType.h
  #SelectSubMatrix.h
  #GetSubmatrix.h
  AppendMatrix.h
  CreateGeometricTransform.h
  ConvertMatrixToScalar.h
  ConvertScalarToMatrix.h
  CreateMatrix.h
  CreateComplexMatrix.h
  EvaluateLinearAlgebraUnary.h
  EvaluateLinearAlgebraBinary.h
  GetMatrixSlice.h
  ReportMatrixInfo.h
  ReportComplexMatrixInfo.h
  ReportMatrixSliceMeasure.h
  share.h
  SolveLinearSystem.h
  SolveComplexLinearSystem.h
  ComputePCA.h
  ConvertRealToComplexMatrix.h
  ConvertComplexToRealMatrix.h
  BooleanCompare.h
  DisplayHistogram.h
  BasicPlotter.h
  AdvancedPlotter.h
  ResizeMatrix.h
  CreateStandardMatrix.h
  ComputeTensorUncertainty.h
)

SCIRUN_ADD_LIBRARY(Modules_Math
  ${Modules_Math_HEADERS}
  ${Modules_Math_SRCS}
)

# Add Python include dirs so Boost.Python's wrap_python.hpp sees pyconfig.h

if (BUILD_WITH_PYTHON)
  set(_PY_INCLUDES "")
  if (DEFINED PYTHON_INCLUDE_DIR AND EXISTS "${PYTHON_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_INCLUDE_DIR}")
  endif()
  if (DEFINED PYTHON_PC_INCLUDE_DIR AND EXISTS "${PYTHON_PC_INCLUDE_DIR}")
    list(APPEND _PY_INCLUDES "${PYTHON_PC_INCLUDE_DIR}")
  endif()
  if (MSVC AND NOT _PY_INCLUDES)
    set(_py_src "${CMAKE_BINARY_DIR}/../Externals/Source/Python_external")
    if (EXISTS "${_py_src}/Include")
      list(APPEND _PY_INCLUDES "${_py_src}/Include")
    endif()
    if (EXISTS "${_py_src}/PC")
      list(APPEND _PY_INCLUDES "${_py_src}/PC")
    endif()
  endif()
  if (_PY_INCLUDES)
    list(REMOVE_DUPLICATES _PY_INCLUDES)
    target_include_directories(Modules_Math PRIVATE ${_PY_INCLUDES})
    message(STATUS "[Modules/Math] Added Python includes: ${_PY_INCLUDES}")
  else()
    message(WARNING "[Modules/Math] Python includes not found at configure time; Boost.Python compilation may fail.")
  endif()

  # Disable Boost.Python autolink so this target doesn't try to link Boost/Python
  target_compile_definitions(Modules_Math PRIVATE BOOST_PYTHON_NO_LIB)          # <-- add this

  # Debug-only: ignore any release python import lib autolink attempts
  if(MSVC)
    target_link_options(Modules_Math PRIVATE
      $<$<CONFIG:Debug>:/NODEFAULTLIB:python313.lib>
    )
  endif()
endif()

TARGET_LINK_LIBRARIES(Modules_Math
  Dataflow_Network
  Core_Datatypes
  Core_Datatypes_Legacy_Field
  Algorithms_Math
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Modules_Math)
ENDIF()

SCIRUN_ADD_TEST_DIR(Tests)