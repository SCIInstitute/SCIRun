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
      const spire::ComponentGroup<ren::StaticTextureMan>& texMan) override
  {
    /// \todo This needs to be moved to pre-execute.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      return;
    }

    if (!srstate.front().state.get(RenderState::IS_TEXT))
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

    // Disable zwrite if we are rendering a transparent object.
    bool depthMask = glIsEnabled(GL_DEPTH_WRITEMASK);
    bool cullFace = glIsEnabled(GL_CULL_FACE);
    bool blend = glIsEnabled(GL_BLEND);

    GL(glEnable(GL_DEPTH_TEST));
    GL(glDepthMask(GL_FALSE));
    GL(glDisable(GL_CULL_FACE));
    GL(glEnable(GL_BLEND));
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    if (rlist.size() > 0)
    {
      glm::mat4 rlistTrafo = trafo.front().transform;

      GLint uniformColorLoc = 0;
      for (const ren::VecUniform& unif : vecUniforms)
      {
        if (std::string(unif.uniformName) == "uColor")
        {
          uniformColorLoc = unif.uniformLocation;
        }
      }

      // Note: Some of this work can be done beforehand. But we elect not to
      // since it is feasible that the data contained in the VBO can change
      // fairly dramatically.

      // Build BSerialize object.
      spire::BSerialize posDeserialize(
          rlist.front().data->getBuffer(), rlist.front().data->getBufferSize());

      spire::BSerialize colorDeserialize(
          rlist.front().data->getBuffer(), rlist.front().data->getBufferSize());

      int64_t posSize     = 0;
      int64_t colorSize   = 0;
      int64_t stride      = 0;  // Stride of entire attributes buffer.

      // Determine stride for our buffer. Also determine appropriate position
      // and color information offsets, and set the offsets. Also determine
      // attribute size in bytes.
      for (const auto& attrib : rlist.front().attributes)
      {
        if (attrib.name == "aPos")
        {
          if (stride != 0) {posDeserialize.readBytes(stride);}
          posSize = attrib.sizeInBytes;
        }
        else if (attrib.name == "aColor")
        {
          if (stride != 0) {colorDeserialize.readBytes(stride);}
          colorSize = attrib.sizeInBytes;
        }

        stride += attrib.sizeInBytes;
      }

      int64_t posStride   = stride - posSize;
      int64_t colorStride = stride - colorSize;

      // Render using a draw list. We will be using the VBO and IBO attached
      // to this object as the basic rendering primitive.
      for (int i = 0; i < rlist.front().numElements; ++i)
      {
        // Read position.
        float x = posDeserialize.read<float>();
        float y = posDeserialize.read<float>();
        float z = posDeserialize.read<float>();
        posDeserialize.readBytes(posStride);

        // Read color if available.
        if (colorSize > 0)
        {
          float r = static_cast<float>(colorDeserialize.read<uint8_t>()) / 255.0f;
          float g = static_cast<float>(colorDeserialize.read<uint8_t>()) / 255.0f;
          float b = static_cast<float>(colorDeserialize.read<uint8_t>()) / 255.0f;
          float a = static_cast<float>(colorDeserialize.read<uint8_t>()) / 255.0f;
          if (colorDeserialize.getBytesLeft() > colorStride)
          {
            colorDeserialize.readBytes(colorStride);
          }
          GL(glUniform4f(uniformColorLoc, r, g, b, a));
        }

        // Update transform.
        rlistTrafo[3].x = x;
        rlistTrafo[3].y = y;
        rlistTrafo[3].z = z;
        commonUniforms.front().applyCommonUniforms(
            rlistTrafo, camera.front().data, time.front().globalTime);

        GL(glDrawElements(ibo.front().primMode, ibo.front().numPrims,
                          ibo.front().primType, 0));
      }
    }
    else
    {
      if (!srstate.front().state.get(RenderState::IS_DOUBLE_SIDED))
      {
        GL(glDrawElements(ibo.front().primMode, ibo.front().numPrims,
                          ibo.front().primType, 0));
      }
      else
      {
        GL(glEnable(GL_CULL_FACE));
        // Double sided rendering. Mimic SCIRun4 and use GL_FRONT and GL_BACK
        // to mimic forward facing and back facing polygons.

        // Draw front facing polygons.
        GLint fdToggleLoc = glGetUniformLocation(shader.front().glid, "uFDToggle");

        GL(glUniform1f(fdToggleLoc, 1.0f));
        glCullFace(GL_BACK);
        GL(glDrawElements(ibo.front().primMode, ibo.front().numPrims,
                          ibo.front().primType, 0));

        GL(glUniform1f(fdToggleLoc, 0.0f));
        glCullFace(GL_FRONT);
        GL(glDrawElements(ibo.front().primMode, ibo.front().numPrims,
                          ibo.front().primType, 0));
      }
    }

    if (depthMask)
    {
      GL(glDepthMask(GL_TRUE));
    }
    if (cullFace)
    {
      GL(glEnable(GL_CULL_FACE));
    }
    if (!blend)
    {
      GL(glDisable(GL_BLEND));
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
