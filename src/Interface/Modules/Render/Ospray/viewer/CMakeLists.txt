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

#INCLUDE(${CMAKE_SOURCE_DIR}/CMake/Modules/FindOSPRay.cmake)
INCLUDE_DIRECTORIES(${OSPRAY_INCLUDE_DIRS})

FIND_PACKAGE(Qt4 REQUIRED QtGui QtOpenGL)
SET(QT_USE_QTOPENGL TRUE)
INCLUDE(${QT_USE_FILE})
SET(LIBS ${LIBS} Qt4::QtGui Qt4::QtOpenGL ${OSPRAY_LIBRARIES})

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS})
SET(LIBS ${LIBS} ${OPENGL_LIBRARIES} ${TBB_LIBRARY_MALLOC} ${TBB_LIBRARY})

SET(SRCS
  #main.cpp
)

ADD_EXECUTABLE(ospVolumeViewer
  ${SRCS}
)

TARGET_LINK_LIBRARIES(ospVolumeViewer
  ${LIBS}
  Interface_Modules_Render_Ospray
)
