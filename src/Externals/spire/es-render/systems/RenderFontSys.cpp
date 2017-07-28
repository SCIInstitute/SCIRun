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

namespace es = CPM_ES_NS;
namespace shaders = CPM_GL_SHADERS_NS;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list. If you need to access
// ESCoreBase, you need to create a static component for it.
namespace ren {

class RenderFontSys : 
    public es::GenericSystem<true,
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
    return es::OptionalComponents<CommonUniforms,
                                  GLState,
                                  StaticGLState,
                                  VecUniform,
                                  MatUniform,
                                  gen::CameraSelect>(type);
  }

  void groupExecute(
      es::ESCoreBase&, uint64_t /* entityID */,
      const es::ComponentGroup<gen::Transform>& trafo,
      const es::ComponentGroup<gen::StaticGlobalTime>& time,
      const es::ComponentGroup<CommonUniforms>& commonUniforms,
      const es::ComponentGroup<VecUniform>& vecUniforms,
      const es::ComponentGroup<MatUniform>& matUniforms,
      const es::ComponentGroup<Shader>& shader,
      const es::ComponentGroup<Texture>& textures,
      const es::ComponentGroup<Font>& font,
      const es::ComponentGroup<RenderFont>& renFontGroup,
      const es::ComponentGroup<GLState>& state,
      const es::ComponentGroup<gen::CameraSelect>& camSelect,
      const es::ComponentGroup<gen::StaticCamera>& camera,
      const es::ComponentGroup<gen::StaticOrthoCamera>& orthoCamera,
      const es::ComponentGroup<StaticGLState>& defaultGLState,
      const es::ComponentGroup<StaticFontMan>& fontMan) override
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

void registerSystem_RenderFont(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<RenderFontSys>();
}

const char* getSystemName_RenderFont()
{
  return RenderFontSys::getName();
}

} // namespace ren

