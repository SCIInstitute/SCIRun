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

# Sources of Core/Algorithms/DataIO classes

SET(Core_Algorithms_Legacy_DataIO_SRCS
  DataIOAlgo.cc
  #MRCReader.cc
  ObjToFieldReader.cc
  VTKToTriSurfReader.cc
  #StreamData/StreamACQFile.cc
)

SET(Core_Algorithms_Legacy_DataIO_HEADERS
  ObjToFieldReader.h
  VTKToTriSurfReader.h
  DataIOAlgo.h
)

#IF(ITK_FOUND)
#  SET(Core_Algorithms_Legacy_DataIO_SRCS
#      ${Core_Algorithms_DataIO_SRCS}
#      AnalyzeImage.cc
#      AnalyzeReader.cc
#      AnalyzeSliceImageIO.cc)
#ENDIF(ITK_FOUND)

SCIRUN_ADD_LIBRARY(Core_Algorithms_Legacy_DataIO
  ${Core_Algorithms_Legacy_DataIO_SRCS}
  ${Core_Algorithms_Legacy_DataIO_HEADERS}
)

TARGET_LINK_LIBRARIES(Core_Algorithms_Legacy_DataIO
  #Core_Algorithms_Util
  Algorithms_Base
  Core_Datatypes
  Core_Datatypes_Legacy_Field
  Core_Datatypes_Legacy_Nrrd
  #Core_Thread
  #Core_Exceptions
  #Core_Geometry
  #Core_Util
  Core_Math
  Core_Persistent
  #Core_Geom
  Core_ImportExport
  #Core_Volume
  #${SCI_TEEM_LIBRARY}
  #${SCI_ZLIB_LIBRARY}
  #${M_LIBRARY}
)

#IF(ITK_FOUND)
#  TARGET_LINK_LIBRARIES(Core_Algorithms_Legacy_DataIO
#    ITKCommon
#    ITKIO
#    ${GDCM_LIBRARY}
#    ${GL_LIBRARY})
#ENDIF(ITK_FOUND)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_Algorithms_Legacy_DataIO)
ENDIF(BUILD_SHARED_LIBS)
