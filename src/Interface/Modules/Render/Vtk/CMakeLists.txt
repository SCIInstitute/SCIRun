SET(SRCS ${SRCS}
  VtkQWidget.cc
  VtkRenderer.cc
)

SET(MOC_HEADERS
  VtkQWidget.h
  VtkRenderer.h
  share.h
)

QT_WRAP_CPP(MOC_OUTFILES "${MOC_HEADERS}")

SCIRUN_ADD_LIBRARY(Interface_Modules_Render_Vtk
  ${SRCS}
  ${MOC_HEADERS}
  ${MOC_OUTFILES}
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
