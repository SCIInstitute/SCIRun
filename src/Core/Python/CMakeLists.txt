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


SET(Core_Python_SRCS
  PythonInterpreter.cc
  PythonDatatypeConverter.cc
)

SET(Core_Python_HEADERS
  PythonInterpreter.h
  PythonDatatypeConverter.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_Python
  ${Core_Python_HEADERS}
  ${Core_Python_SRCS}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Python)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)

IF(WIN32)
  ADD_DEFINITIONS(-DPYTHONPATH=L"${SCI_PYTHON_MODULE_PARENT_PATH}"
                  -DPYTHONNAME=L"${SCI_PYTHON_NAME}")
ELSEIF(APPLE)
  ADD_DEFINITIONS(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}"
                  -DPYTHONLIBDIR=L"${SCI_PYTHON_LIBRARY_DIR}"
                  -DPYTHONLIB=L"${SCI_PYTHON_LIBRARY}")
ELSE()
  ADD_DEFINITIONS(-DPYTHONPATH=L"${PYTHON_MODULE_SEARCH_PATH}")
ENDIF()

TARGET_LINK_LIBRARIES(Core_Python
  ${SCI_PYTHON_LIBRARY}
  ${SCI_BOOST_LIBRARY}
  SCIRunPythonAPI
  Core_Matlab
  Core_Datatypes
  Core_Datatypes_Legacy_Field
)

SET_TARGET_PROPERTIES(Core_Python
  PROPERTIES
  COMPILE_DEFINITIONS_DEBUG "BOOST_DEBUG_PYTHON"
)
