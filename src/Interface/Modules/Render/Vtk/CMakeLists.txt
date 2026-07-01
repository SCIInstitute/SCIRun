SET(QT_USE_QTOPENGL TRUE)
SET(LIBS ${LIBS} ${QT_GRAPHICS_LIBRARIES} ${QT_LIBRARIES})

FIND_PACKAGE(OpenGL REQUIRED)
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIRS})
SET(LIBS ${LIBS} ${OPENGL_LIBRARIES} ${TBB_LIBRARY_MALLOC} ${TBB_LIBRARY})

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
  VTK::FiltersSources
  VTK::FiltersCore
  VTK::RenderingCore
  VTK::RenderingOpenGL2
  VTK::RenderingAnnotation
  VTK::InteractionStyle
  ${SCI_SPIRE_LIBRARY}
)

vtk_module_autoinit(
TARGETS Interface_Modules_Render_Vtk
MODULES
    VTK::RenderingOpenGL2
    VTK::InteractionStyle
)

IF(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_Interface_Modules_Render_Vtk)
ENDIF(BUILD_SHARED_LIBS)
