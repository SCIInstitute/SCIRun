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


# CMakeLists.txt for Core/Matlab

SET(Core_Matlab_SRCS
  fieldtomatlab.cc
  matfile.cc
  matfiledata.cc
  matlabarray.cc
  matlabconverter.cc
  matlabfile.cc
  matlabtofield.cc
)

SET(Core_Matlab_HEADERS
  fieldtomatlab.h
  matfile.h
  matfilebase.h
  matfiledata.h
  matfiletemplate.h
  matlabarray.h
  matlabconverter.h
  matlabfile.h
  MatlabIO.h
  matlabtofield.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_Matlab ${Core_Matlab_SRCS} ${Core_Matlab_HEADERS})

TARGET_LINK_LIBRARIES(Core_Matlab
  Core_Datatypes
  Core_Datatypes_Legacy_Field
  Core_Datatypes_Legacy_Nrrd
  #Core_Exceptions
  #Core_Thread
  Core_Geometry_Primitives
  #Core_Util
  Core_Math
  Core_Logging
  ${SCI_ZLIB_LIBRARY}
  ${SCI_TEEM_LIBRARY}
)

IF(MATLAB_STRING_OUTPUT_SIGNED_TYPE_FIX_FOR_SCIPY)
  ADD_DEFINITIONS(-DMATLAB_STRING_OUTPUT_SIGNED_TYPE_FIX_FOR_SCIPY)
ENDIF()

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Matlab)
ENDIF(BUILD_SHARED_LIBS)
