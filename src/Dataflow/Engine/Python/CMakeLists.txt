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

SET(SCIRunPythonAPI_SRCS
  NetworkEditorPythonAPI.cc
)

SET(SCIRunPythonAPI_HEADERS
  NetworkEditorPythonAPI.h
  NetworkEditorPythonInterface.h
  SCIRunPythonModule.h
  share.h
)

SCIRUN_ADD_LIBRARY(SCIRunPythonAPI 
  ${SCIRunPythonAPI_HEADERS}
  ${SCIRunPythonAPI_SRCS}
)

TARGET_LINK_LIBRARIES(SCIRunPythonAPI
  Core_Thread
  ${SCI_PYTHON_LIBRARY}
  ${SCI_BOOST_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_SCIRunPythonAPI)
    
IF(WIN32)
  SET_TARGET_PROPERTIES(SCIRunPythonAPI
    PROPERTIES
    IMPORT_SUFFIX ".pyd"
    IMPORT_PREFIX ""
    PREFIX ""
    SUFFIX ".pyd"
  )
ENDIF()
    
ENDIF(BUILD_SHARED_LIBS)

SCIRUN_ADD_TEST_DIR(Tests)

