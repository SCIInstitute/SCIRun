SET(SRCS ${SRCS}
  VtkRenderer.cpp
)

SET(MOC_HEADERS
  VtkRenderer.h
  share.h
)

SCIRUN_ADD_LIBRARY(Interface_Modules_Render_Vtk
  ${SRCS}
  ${MOC_HEADERS}
)

TARGET_LINK_LIBRARIES(Interface_Modules_Render_Vtk
  Interface_Modules_Base
  Core_Algorithms_Visualization
  Core_Datatypes
  Modules_Render
  ${LIBS}
  VTK::CommonCore
  ${SCI_SPIRE_LIBRARY}
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Render_Vtk)
ENDIF(BUILD_SHARED_LIBS)
