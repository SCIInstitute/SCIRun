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
#include <es-general/comp/StaticOrthoCamera.hpp>
#include <es-general/comp/CameraSelect.hpp>

#include "../comp/VBO.hpp"
#include "../comp/IBO.hpp"
#include "../comp/CommonUniforms.hpp"
#include "../comp/Shader.hpp"
#include "../comp/Texture.hpp"
#include "../comp/VecUniform.hpp"
#include "../comp/MatUniform.hpp"
#include "../comp/GLState.hpp"
#include "../comp/StaticVBOMan.hpp"
#include "../comp/StaticFontMan.hpp"
#include "../comp/StaticGLState.hpp"
#include "../comp/RenderFont.hpp"
#include "../comp/Font.hpp"

namespace es = spire;
namespace shaders = spire;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list. If you need to access
// ESCoreBase, you need to create a static component for it.
namespace ren {

class RenderFontSys :
    public spire::GenericSystem<true,
                             gen::Transform,
                             gen::StaticGlobalTime,
                             CommonUniforms,
                             VecUniform,
                             MatUniform,
                             Shader,
                             Texture,
                             Font,
                             RenderFont,
                             GLState,
                             gen::CameraSelect,
                             gen::StaticCamera,
                             gen::StaticOrthoCamera,
                             StaticGLState,
                             StaticFontMan>
{
public:

  static const char* getName() {return "ren:RenderZFontSys";}

  bool isComponentOptional(uint64_t type) override
  {
    return spire::OptionalComponents<CommonUniforms,
                                  GLState,
                                  StaticGLState,
                                  VecUniform,
                                  MatUniform,
                                  gen::CameraSelect>(type);
  }

  void groupExecute(
      spire::ESCoreBase&, uint64_t /* entityID */,
      const spire::ComponentGroup<gen::Transform>& trafo,
      const spire::ComponentGroup<gen::StaticGlobalTime>& time,
      const spire::ComponentGroup<CommonUniforms>& commonUniforms,
      const spire::ComponentGroup<VecUniform>& vecUniforms,
      const spire::ComponentGroup<MatUniform>& matUniforms,
      const spire::ComponentGroup<Shader>& shader,
      const spire::ComponentGroup<Texture>& textures,
      const spire::ComponentGroup<Font>& font,
      const spire::ComponentGroup<RenderFont>& renFontGroup,
      const spire::ComponentGroup<GLState>& state,
      const spire::ComponentGroup<gen::CameraSelect>& camSelect,
      const spire::ComponentGroup<gen::StaticCamera>& camera,
      const spire::ComponentGroup<gen::StaticOrthoCamera>& orthoCamera,
      const spire::ComponentGroup<StaticGLState>& defaultGLState,
      const spire::ComponentGroup<StaticFontMan>& fontMan) override
  {
    const RenderFont& renFont = renFontGroup.front();

    // Setup *everything*. We don't want to enter multiple conditional
    // statements if we can avoid it. So we assume everything has not been
    // setup (including uniforms) if the simple geom hasn't been setup.
    if (renFont.isSetUp() == false)
    {
      // Build font geometry from text string inside of renFont.
      const_cast<RenderFont&>(renFont).constructFontGeometry(
          fontMan.front(), font.front().fontID);

      // Build pre-applied attributes.
      const_cast<RenderFont&>(renFont).setupAttributesAndUniforms(shader.front().glid);

      /// \todo Optimize by pulling uniforms only once.
      if (commonUniforms.size() > 0)
        const_cast<CommonUniforms&>(commonUniforms.front()).checkUniformArray(
            shader.front().glid);

      if (vecUniforms.size() > 0)
      {
        for (const VecUniform& unif : vecUniforms)
        {
          const_cast<VecUniform&>(unif).checkUniform(shader.front().glid);
        }
      }

      if (matUniforms.size() > 0)
      {
        for (const MatUniform& unif : matUniforms)
        {
          const_cast<MatUniform&>(unif).checkUniform(shader.front().glid);
        }
      }
    }

    if (textures.size() > 0)
    {
      for (const Texture& tex : textures)
      {
        if (tex.isSetUp() == false)
        {
          const_cast<Texture&>(tex).checkUniform(shader.front().glid);
          if (tex.isSetUp() == false)
          {
            return;
          }
        }
      }
    }

    if (state.size() > 0 && defaultGLState.size() > 0)
    {
      state.front().state.applyRelative(defaultGLState.front().state);
    }

    // Bind shader.
    GL(glUseProgram(shader.front().glid));

    // Bind VBO and IBO
    GL(glBindBuffer(GL_ARRAY_BUFFER, renFont.vbo));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renFont.ibo));

    // Bind any common uniforms.
    if (commonUniforms.size() > 0)
    {
      gen::CameraSelect::Selection sel = gen::CameraSelect::PERSPECTIVE_CAMERA;

      if (camSelect.size() > 0)
        sel = camSelect.front().cam;

      if (sel == gen::CameraSelect::PERSPECTIVE_CAMERA)
        commonUniforms.front().applyCommonUniforms(
            trafo.front().transform, camera.front().data, time.front().globalTime);
      else
        commonUniforms.front().applyCommonUniforms(
            trafo.front().transform, orthoCamera.front().data, time.front().globalTime);
    }

    // Apply textures.
    for (const Texture& tex : textures) tex.applyUniform();

    // Apply vector uniforms (if any).
    for (const VecUniform& unif : vecUniforms) unif.applyUniform();

    // Apply matrix uniforms (if any).
    for (const MatUniform& unif : matUniforms) unif.applyUniform();

    // Apply font specific uniforms.
    renFont.applyUniforms();

    shaders::bindPreappliedAttrib(renFont.appliedAttribs,
                                  static_cast<size_t>(renFont.attribSize),
                                  renFont.stride);

    GL(glDrawElements(renFont.primMode, renFont.numPrims,
                      renFont.primType, 0));

    shaders::unbindPreappliedAttrib(renFont.appliedAttribs,
                                    static_cast<size_t>(renFont.attribSize));

    if (state.size() > 0 && defaultGLState.size() > 0)
    {
      defaultGLState.front().state.applyRelative(state.front().state);
    }
  }
};

void registerSystem_RenderFont(spire::Acorn& core)
{
  core.registerSystem<RenderFontSys>();
}

const char* getSystemName_RenderFont()
{
  return RenderFontSys::getName();
}

} // namespace ren
