/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
   University of Utah.

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/


#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>
#include <es-render/comp/StaticTextureMan.hpp>

#include "RenderTransCommon.h"

namespace es = spire;
namespace shaders = spire;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list.

namespace SCIRun {
namespace Render {

class RenderTransText :
    public spire::GenericSystem<true,
                             RenderBasicGeom,   // TAG class
                             SRRenderState,
                             RenderList,
                             LightingUniforms,
                             ClippingPlaneUniforms,
                             gen::Transform,
                             gen::StaticGlobalTime,
                             ren::VBO,
                             ren::IBO,
                             ren::Texture,
                             ren::CommonUniforms,
                             ren::VecUniform,
                             ren::MatUniform,
                             ren::Shader,
                             ren::GLState,
                             StaticWorldLight,
                             StaticClippingPlanes,
                             gen::StaticCamera,
                             ren::StaticGLState,
                             ren::StaticVBOMan,
                             ren::StaticTextureMan>
{
public:

  static const char* getName() {return "RenderTransText";}

  bool isComponentOptional(uint64_t type) override
  {
    return spire::OptionalComponents<RenderList,
                                  ren::GLState,
                                  ren::StaticGLState,
                                  ren::CommonUniforms,
                                  LightingUniforms,
                                  ClippingPlaneUniforms,
                                  ren::VecUniform,
                                  ren::MatUniform,
                                  ren::Texture,
                                  ren::StaticTextureMan>(type);
  }

  void groupExecute(
      spire::ESCoreBase&, uint64_t /* entityID */,
      const spire::ComponentGroup<RenderBasicGeom>& geom,
      const spire::ComponentGroup<SRRenderState>& srstate,
      const spire::ComponentGroup<RenderList>& rlist,
      const spire::ComponentGroup<LightingUniforms>& lightUniforms,
      const spire::ComponentGroup<ClippingPlaneUniforms>& clippingPlaneUniforms,
      const spire::ComponentGroup<gen::Transform>& trafo,
      const spire::ComponentGroup<gen::StaticGlobalTime>& time,
      const spire::ComponentGroup<ren::VBO>& vbo,
      const spire::ComponentGroup<ren::IBO>& ibo,
      const spire::ComponentGroup<ren::Texture>& textures,
      const spire::ComponentGroup<ren::CommonUniforms>& commonUniforms,
      const spire::ComponentGroup<ren::VecUniform>& vecUniforms,
      const spire::ComponentGroup<ren::MatUniform>& matUniforms,
      const spire::ComponentGroup<ren::Shader>& shader,
      const spire::ComponentGroup<ren::GLState>& state,
      const spire::ComponentGroup<StaticWorldLight>& worldLight,
      const spire::ComponentGroup<StaticClippingPlanes>& clippingPlanes,
      const spire::ComponentGroup<gen::StaticCamera>& camera,
      const spire::ComponentGroup<ren::StaticGLState>& defaultGLState,
      const spire::ComponentGroup<ren::StaticVBOMan>& vboMan,
      const spire::ComponentGroup<ren::StaticTextureMan>&) override
  {
    /// \todo This needs to be moved to pre-execute.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      return;
    }

    if (!srstate.front().state.get(RenderState::ActionFlags::IS_TEXT))
    {
      return;
    }

    GLuint iboID = ibo.front().glid;

    renderTransparentGeometry(geom, srstate, rlist, lightUniforms,
        clippingPlaneUniforms, trafo, time, vbo, ibo, textures, commonUniforms,
        vecUniforms, matUniforms, shader, state, worldLight, clippingPlanes,
        camera, defaultGLState, vboMan, iboID);
  }
};

void registerSystem_RenderTransTextGeom(spire::Acorn& core)
{
  core.registerSystem<RenderTransText>();
}

const char* getSystemName_RenderTransTextGeom()
{
  return RenderTransText::getName();
}

} // namespace Render
} // namespace SCIRun
