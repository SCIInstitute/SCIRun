## ======================================================================== ##
## Copyright 2009-2017 Intel Corporation                                    ##
##                                                                          ##
## Licensed under the Apache License, Version 2.0 (the "License");          ##
## you may not use this file except in compliance with the License.         ##
## You may obtain a copy of the License at                                  ##
##                                                                          ##
##     http://www.apache.org/licenses/LICENSE-2.0                           ##
##                                                                          ##
## Unless required by applicable law or agreed to in writing, software      ##
## distributed under the License is distributed on an "AS IS" BASIS,        ##
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. ##
## See the License for the specific language governing permissions and      ##
## limitations under the License.                                           ##
## ======================================================================== ##


SET(QT_USE_QTOPENGL TRUE)
SET(LIBS ${LIBS} Qt5::OpenGL ${QT_LIBRARIES})

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS})
SET(LIBS ${LIBS} ${OPENGL_LIBRARIES} ${TBB_LIBRARY_MALLOC} ${TBB_LIBRARY})

SET(SRCS ${SRCS}
  QOSPRayWidget.cpp
  OSPRayRenderer.cpp
  OSPRayDataManager.cpp
  OSPRayCamera.cpp
  #util.h
  #util.cpp
  #importer/Importer.h
  #importer/Importer.cpp
  #importer/importOSP.cpp
  #importer/importRAW.cpp
  #importer/importRM.cpp
  #importer/TinyXML2.cpp
  #importer/TinyXML2.h
)

SET(MOC_HEADERS
  QOSPRayWidget.h
  OSPRayRenderer.h
  OSPRayDataManager.h
  OSPRayCamera.h
  share.h
)

QT_WRAP_CPP(MOC_OUTFILES "${MOC_HEADERS}")

SCIRUN_ADD_LIBRARY(Interface_Modules_Render_Ospray
  ${SRCS}
  ${MOC_HEADERS}
  ${MOC_OUTFILES}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Render_Ospray
  Interface_Modules_Base
  Core_Algorithms_Visualization
  Modules_Render
  ${LIBS}
  ospray::ospray
  ${SCI_SPIRE_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Render_Ospray)
ENDIF(BUILD_SHARED_LIBS)

#Disable standalone viewer for now
#ADD_SUBDIRECTORY(viewer)
