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

#include <glm/glm.hpp>
#include <gl-platform/GLPlatform.hpp>
#include <entity-system/GenericSystem.hpp>
#include <es-systems/SystemCore.hpp>
#include <es-acorn/Acorn.hpp>

#include <es-general/comp/Transform.hpp>
#include <es-general/comp/StaticGlobalTime.hpp>
#include <es-general/comp/StaticCamera.hpp>

#include <es-render/comp/VBO.hpp>
#include <es-render/comp/IBO.hpp>
#include <es-render/comp/CommonUniforms.hpp>
#include <es-render/comp/Shader.hpp>
#include <es-render/comp/Texture.hpp>
#include <es-render/comp/GLState.hpp>
#include <es-render/comp/VecUniform.hpp>
#include <es-render/comp/MatUniform.hpp>
#include <es-render/comp/StaticGLState.hpp>
#include <es-render/comp/StaticVBOMan.hpp>
#include <es-render/comp/StaticTextureMan.hpp>

#include <bserialize/BSerialize.hpp>

#include "../comp/RenderBasicGeom.h"
#include "../comp/SRRenderState.h"
#include "../comp/RenderList.h"
#include "../comp/StaticWorldLight.h"
#include "../comp/StaticClippingPlanes.h"
#include "../comp/LightingUniforms.h"
#include "../comp/ClippingPlaneUniforms.h"

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list.

namespace SCIRun {
namespace Render {

class RenderBasicSys :
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

  static const char* getName() {return "RenderBasicSys";}

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
      const spire::ComponentGroup<ren::StaticTextureMan>& texMan) override
  {
    /// \todo This needs to be moved to pre-execute.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      return;
    }

    if (srstate.front().state.get(RenderState::USE_TRANSPARENCY) ||
        srstate.front().state.get(RenderState::USE_TRANSPARENT_EDGES) ||
        srstate.front().state.get(RenderState::USE_TRANSPARENT_NODES) ||
        srstate.front().state.get(RenderState::IS_TEXT))
    {
      return;
    }

    GLuint iboID = ibo.front().glid;

    // Setup *everything*. We don't want to enter multiple conditional
    // statements if we can avoid it. So we assume everything has not been
    // setup (including uniforms) if the simple geom hasn't been setup.
    if (!geom.front().attribs.isSetup())
    {
      // We use const cast to get around a 'modify' call for 2 reasons:
      // 1) This is populating system specific GL data. It has no bearing on the
      //    actual simulation state.
      // 2) It is more correct than issuing a modify call. The data is used
      //    directly below to render geometry.
      const_cast<RenderBasicGeom&>(geom.front()).attribs.setup(
          vbo.front().glid, shader.front().glid, vboMan.front());

      /// \todo Optimize by pulling uniforms only once.
      if (commonUniforms.size() > 0)
      {
        const_cast<ren::CommonUniforms&>(commonUniforms.front()).checkUniformArray(
            shader.front().glid);
      }

      if (vecUniforms.size() > 0)
      {
        for (const ren::VecUniform& unif : vecUniforms)
        {
          const_cast<ren::VecUniform&>(unif).checkUniform(shader.front().glid);
        }
      }

      if (matUniforms.size() > 0)
      {
        for (const ren::MatUniform& unif : matUniforms)
        {
          const_cast<ren::MatUniform&>(unif).checkUniform(shader.front().glid);
        }
      }

      if (lightUniforms.size() > 0)
        const_cast<LightingUniforms&>(lightUniforms.front()).checkUniformArray(shader.front().glid);

      if (clippingPlaneUniforms.size() > 0)
        const_cast<ClippingPlaneUniforms&>(clippingPlaneUniforms.front()).checkUniformArray(shader.front().glid);
    }

    // Check to see if we have GLState. If so, apply it relative to the
    // current state (I'm actually thinking GLState is a bad idea, and we
    // should just program what we need manually in the system -- depending
    // on type). State can be set in a pre-walk phase.
    if (state.size() > 0 && defaultGLState.size() > 0)
    {
      // Apply GLState based on current GLState (the static state), if it is
      // present. Otherwise, fully apply it (performance issue).
      state.front().state.applyRelative(defaultGLState.front().state);
    }

    // Bind shader.
    GL(glUseProgram(shader.front().glid));

    // Bind VBO and IBO
    GL(glBindBuffer(GL_ARRAY_BUFFER, vbo.front().glid));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID));

    bool depthMask = glIsEnabled(GL_DEPTH_WRITEMASK);
    bool cullFace = glIsEnabled(GL_CULL_FACE);
    bool blend = glIsEnabled(GL_BLEND);

    GL(glDepthMask(GL_TRUE));
    GL(glDisable(GL_CULL_FACE));
    GL(glDisable(GL_BLEND));

    // Bind any common uniforms.
    if (commonUniforms.size() > 0)
    {
      commonUniforms.front().applyCommonUniforms(
          trafo.front().transform, camera.front().data, time.front().globalTime);
    }

    // Apply vector uniforms (if any).
    for (const ren::VecUniform& unif : vecUniforms) {unif.applyUniform();}
    if (lightUniforms.size() > 0)
    {
      std::vector<glm::vec3> lightDir(worldLight.front().lightDir,
        worldLight.front().lightDir + LIGHT_NUM);
      std::vector<glm::vec3> lightColor(worldLight.front().lightColor,
        worldLight.front().lightColor + LIGHT_NUM);
      lightUniforms.front().applyUniform(lightDir, lightColor);
    }
    if (clippingPlaneUniforms.size() > 0)
    {
      glm::mat4 transform = trafo.front().transform;
      clippingPlaneUniforms.front().applyUniforms(transform, clippingPlanes.front().clippingPlanes,
      clippingPlanes.front().clippingPlaneCtrls);
    }

    // Apply matrix uniforms (if any).
    for (const ren::MatUniform& unif : matUniforms) {unif.applyUniform();}

    // bind textures
    for (const ren::Texture& tex : textures)
    {
      GL(glActiveTexture(GL_TEXTURE0 + tex.textureUnit));
      GL(glBindTexture(tex.textureType, tex.glid));
    }

    geom.front().attribs.bind();

    GL(glDrawElements(ibo.front().primMode, ibo.front().numPrims, ibo.front().primType, 0));

    if (!depthMask)
    {
      GL(glDepthMask(GL_FALSE));
    }
    if (cullFace)
    {
      GL(glEnable(GL_CULL_FACE));
    }
    if (blend)
    {
      GL(glEnable(GL_BLEND));
    }

    // unbind textures
    for (const ren::Texture& tex : textures)
    {
      GL(glActiveTexture(GL_TEXTURE0 + tex.textureUnit));
      GL(glBindTexture(tex.textureType, 0));
    }

    geom.front().attribs.unbind();

    // Reapply the default state here -- only do this if static state is
    // present.
    if (state.size() > 0 && defaultGLState.size() > 0)
    {
      defaultGLState.front().state.applyRelative(state.front().state);
    }
  }
};

void registerSystem_RenderBasicGeom(spire::Acorn& core)
{
  core.registerSystem<RenderBasicSys>();
}

const char* getSystemName_RenderBasicGeom()
{
  return RenderBasicSys::getName();
}

} // namespace Render
} // namespace SCIRun
