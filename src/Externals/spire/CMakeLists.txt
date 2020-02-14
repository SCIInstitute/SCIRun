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


MESSAGE(STATUS "Configuring Spire Meta-library")

PROJECT ( Spire )

IF (WIN32)
  ADD_DEFINITIONS ( -D_CRT_SECURE_NO_WARNINGS )
ENDIF (WIN32)

IF ( CMAKE_BUILD_TYPE MATCHES "Debug" )
  SET ( SPIRE_LIBRARY_NAME "spireD" )
ELSE ( CMAKE_BUILD_TYPE MATCHES "Debug" )
  SET ( SPIRE_LIBRARY_NAME "spire" )
ENDIF ( CMAKE_BUILD_TYPE MATCHES "Debug" )

SET(BUILD_SHARED_LIBS OFF)

SET(SCI_SPIRE_LIBRARY ${SPIRE_LIBRARY_NAME} CACHE INTERNAL "spire library name." FORCE)

ADD_DEFINITIONS(-DGLM_SWIZZLE -DGLM_FORCE_CXX03 -DGLM_FORCE_RADIANS)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

IF(UNIX)
  ADD_DEFINITIONS(-Wno-missing-braces -fPIC)
ENDIF()

ADD_LIBRARY ( ${SCI_SPIRE_LIBRARY}
  arc-ball/ArcBall.hpp
  arc-look-at/ArcLookAt.hpp
  bserialize/BSerialize.hpp
  bserialize/src/BSerializeDetail.hpp
  cereal-glm/CerealGLM.hpp
  entity-system/BaseSystem.hpp
  entity-system/DuplicateComponent.hpp
  entity-system/EmptySystem.hpp
  entity-system/ESCore.hpp
  entity-system/ESCoreBase.hpp
  entity-system/GenericSystem.hpp
  entity-system/src/BaseComponentContainer.hpp
  entity-system/src/ComponentContainer.hpp
  entity-system/src/ComponentGroup.hpp
  entity-system/src/EmptyComponentContainer.hpp
  entity-system/src/TemplateID.hpp
  es-acorn/Acorn.hpp
  es-cereal/CerealCore.hpp
  es-cereal/CerealHeap.hpp
  es-cereal/CerealTypeSerialize.hpp
  es-cereal/ComponentSerialize.hpp
  #es-fs/emscripten/FilesystemEm.hpp
  es-fs/Filesystem.hpp
  es-fs/FilesystemDummy.hpp
  es-fs/FilesystemSync.hpp
  es-fs/fscomp/StaticFS.hpp
  es-fs/Registration.hpp
  es-general/AABB2D.hpp
  es-general/comp/CameraSelect.hpp
  es-general/comp/ClickBox2D.hpp
  es-general/comp/ConstantRotation.hpp
  es-general/comp/StaticCamera.hpp
  es-general/comp/StaticGlobalTime.hpp
  es-general/comp/StaticKeyboardInput.hpp
  es-general/comp/StaticMouseInput.hpp
  es-general/comp/StaticObjRefID.hpp
  es-general/comp/StaticOrthoCamera.hpp
  es-general/comp/StaticRandom.hpp
  es-general/comp/StaticScreenDims.hpp
  es-general/comp/Transform.hpp
  es-general/Registration.hpp
  es-general/systems/ClickBox2DSys.hpp
  es-general/systems/ConstantRotationSys.hpp
  es-general/util/Math.hpp
  es-render/BMFont.hpp
  es-render/comp/AssetPromise.hpp
  es-render/comp/CommonUniforms.hpp
  es-render/comp/FBO.hpp
  es-render/comp/Font.hpp
  es-render/comp/FontPromise.hpp
  es-render/comp/Geom.hpp
  es-render/comp/GeomPromise.hpp
  es-render/comp/GLState.hpp
  es-render/comp/IBO.hpp
  es-render/comp/MatUniform.hpp
  es-render/comp/RenderFont.hpp
  es-render/comp/RenderSequence.hpp
  es-render/comp/RenderSimpleGeom.hpp
  es-render/comp/Shader.hpp
  es-render/comp/ShaderPromiseVF.hpp
  es-render/comp/SkinnedGeom.hpp
  es-render/comp/StaticArcBallCam.hpp
  es-render/comp/StaticFBOMan.hpp
  es-render/comp/StaticFontMan.hpp
  es-render/comp/StaticGeomMan.hpp
  es-render/comp/StaticGLState.hpp
  es-render/comp/StaticIBOMan.hpp
  es-render/comp/StaticShaderMan.hpp
  es-render/comp/StaticTextureMan.hpp
  es-render/comp/StaticVBOMan.hpp
  es-render/comp/Texture.hpp
  es-render/comp/TexturePromise.hpp
  es-render/comp/UniformLocation.hpp
  es-render/comp/UtilViewPosAlign.hpp
  es-render/comp/VBO.hpp
  es-render/comp/VecUniform.hpp
  es-render/FBOMan.hpp
  es-render/FontMan.hpp
  es-render/GeomMan.hpp
  es-render/IBOMan.hpp
  es-render/Registration.hpp
  es-render/ShaderMan.hpp
  es-render/systems/ArcBallCameraSys.hpp
  es-render/systems/debug/RenderClickBox2DSys.hpp
  es-render/systems/RenderFontSys.hpp
  es-render/systems/RenderSimpleGeomSys.hpp
  es-render/systems/UtilViewPosAlignSys.hpp
  es-render/TextureMan.hpp
  es-render/util/Camera.hpp
  es-render/util/DebugRender.hpp
  es-render/util/helper/TypeToGLUniform.hpp
  es-render/util/Lines.hpp
  es-render/util/Shader.hpp
  es-render/util/TexQuad.hpp
  es-render/util/Uniform.hpp
  es-render/VBOMan.hpp
  es-systems/SystemCore.hpp
  es-systems/SystemFactory.hpp
  gl-platform/GLPlatform.hpp
  gl-shaders/GLShader.hpp
  gl-state/GLState.hpp
  glm-aabb/AABB.hpp
  var-buffer/VarBuffer.hpp

  arc-ball/ArcBall.cpp
  arc-look-at/ArcLookAt.cpp
  bserialize/BSerialize.cpp
  entity-system/ESCoreBase.cpp
  entity-system/src/BaseComponentContainer.cpp
  entity-system/src/TemplateID.cpp
  es-acorn/Acorn.cpp
  es-cereal/CerealCore.cpp
  es-cereal/CerealHeap.cpp
  es-cereal/CerealTypeSerialize.cpp
  es-cereal/ComponentSerialize.cpp
  #es-fs/emscripten/FilesystemEm.cpp
  es-fs/Filesystem.cpp
  es-fs/FilesystemDummy.cpp
  es-fs/FilesystemSync.cpp
  es-fs/Registration.cpp
  es-general/Registration.cpp
  es-general/systems/ClickBox2DSys.cpp
  es-general/systems/ConstantRotationSys.cpp
  es-general/util/Math.cpp
  es-render/BMFont.cpp
  es-render/comp/CommonUniforms.cpp
  es-render/comp/MatUniform.cpp
  es-render/comp/RenderFont.cpp
  es-render/comp/RenderSimpleGeom.cpp
  es-render/comp/Texture.cpp
  es-render/comp/VecUniform.cpp
  es-render/FBOMan.cpp
  es-render/FontMan.cpp
  es-render/GeomMan.cpp
  es-render/IBOMan.cpp
  es-render/Registration.cpp
  es-render/Registration1.cpp
  es-render/Registration2.cpp
  es-render/Registration3.cpp
  es-render/ShaderMan.cpp
  es-render/systems/ArcBallCameraSys.cpp
  es-render/systems/debug/RenderClickBox2DSys.cpp
  es-render/systems/RenderFontSys.cpp
  es-render/systems/RenderSimpleGeomSys.cpp
  es-render/systems/UtilViewPosAlignSys.cpp
  es-render/TextureMan.cpp
  es-render/util/Camera.cpp
  es-render/util/DebugRender.cpp
  es-render/util/Lines.cpp
  es-render/util/Shader.cpp
  es-render/util/TexQuad.cpp
  es-render/VBOMan.cpp
  es-systems/SystemCore.cpp
  es-systems/SystemFactory.cpp
  gl-platform/GLPlatform.cpp
  gl-shaders/GLShader.cpp
  gl-state/GLState.cpp
  glm-aabb/AABB.cpp
  var-buffer/VarBuffer.cpp
  #es-log/trace-log.h
  #es-log/trace-log.cc
  scishare.h
  )

target_link_libraries(${SCI_SPIRE_LIBRARY}
  ${SCI_LODEPNG_LIBRARY}
  ${SCI_TNY_LIBRARY}
  ${OPENGL_LIBRARIES})

IF(WIN32)
  target_link_libraries(${SCI_SPIRE_LIBRARY} ${SCI_GLEW_LIBRARY})
ENDIF()

IF(BUILD_SHARED_LIBS)
	ADD_DEFINITIONS(-DBUILD_spire_ES)
ENDIF(BUILD_SHARED_LIBS)
