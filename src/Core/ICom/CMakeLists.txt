#
#  For more information, please see: http://software.sci.utah.edu
#
#  The MIT License
#
#  Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

# Sources of Core/ICom classes
SET(Core_ICom_SRCS
  IComAddress.cc
  IComPacket.cc
  IComSocket.cc
  IComVirtualSocket.cc
  IComINetSocket.cc
  IComInternalSocket.cc
  #IComSslSocket.cc
  #IComRHostList.cc
)

SET(Core_ICom_HEADERS
  IComAddress.h
  IComPacket.h
  IComSocket.h
  IComVirtualSocket.h
  IComINetSocket.h
  IComInternalSocket.h
  IComSslSocket.h
  IComRHostList.h
  IComBase.h
  IComFwd.h
  share.h
)

SCIRUN_ADD_LIBRARY(Core_ICom ${Core_ICom_SRCS} ${Core_ICom_HEADERS})

########################################################################
# Socket library
# DV Not sure if socket library needs to be defined for cygwin

IF(WIN32 AND NOT CYGWIN)
  SET(SOCKET_LIBRARY ws2_32 CACHE INTERNAL "Socket library ws2_32")
  SET(THREAD_LIBRARY winmm CACHE INTERNAL "Time functionality library winmm")
ENDIF(WIN32 AND NOT CYGWIN)

TARGET_LINK_LIBRARIES(Core_ICom
  Core_Thread
  #Core_Util
  #Core_Exceptions
  ${SOCKET_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Core_ICom)
ENDIF(BUILD_SHARED_LIBS)
