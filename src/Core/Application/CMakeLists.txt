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


SET(Core_Application_SRCS
  Application.cc
  ${version_file}
)

SET(Core_Application_HEADERS
  Application.h
  Version.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_Application
  ${Core_Application_HEADERS}
  ${Core_Application_SRCS}
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
    target_include_directories(Core_Application PRIVATE ${_PY_INCLUDES})
    message(STATUS "[Core/Application] Added Python includes: ${_PY_INCLUDES}")
  else()
    message(WARNING "[Core/Application] Python includes not found at configure time; Boost.Python compilation may fail.")
  endif()
endif()

TARGET_LINK_LIBRARIES(Core_Application
  Core_Command
  Core_CommandLine
  Engine_Network
  Engine_Scheduler
  Modules_Factory
  Algorithms_Factory
  Dataflow_State
  Core_Logging
  Core_IEPlugin
  Core_Application_Session
  Core_Application_Preferences
  ${SCI_BOOST_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Application)
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)

ADD_SUBDIRECTORY(Preferences)
ADD_SUBDIRECTORY(Session)
