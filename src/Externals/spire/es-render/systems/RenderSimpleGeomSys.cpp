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
#include "../comp/RenderSimpleGeom.hpp"
#include "../comp/CommonUniforms.hpp"
#include "../comp/Shader.hpp"
#include "../comp/Texture.hpp"
#include "../comp/GLState.hpp"
#include "../comp/VecUniform.hpp"
#include "../comp/MatUniform.hpp"
#include "../comp/StaticGLState.hpp"
#include "../comp/StaticVBOMan.hpp"

namespace es = CPM_ES_NS;
namespace shaders = CPM_GL_SHADERS_NS;

/// \note This is a *very* generic renderer. Renderers based on this can be
///       made and specialized. See the font renderer. We could also specialize
///       onto a quad renderer. Or an orthographic-only quad renderer. These
///       specializations would speed up rendering time for the affected
///       objects and would likely lead to better cache locality on the GPU.

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list. If you need to access
// ESCoreBase, you need to create a static component for it.
namespace ren {

class RenderSimpleGeomSys : 
    public es::GenericSystem<true,
                             gen::Transform,
                             gen::StaticGlobalTime,
                             VBO,
                             IBO,
                             RenderSimpleGeom,
                             CommonUniforms,
                             VecUniform,
                             MatUniform,
                             Shader,
                             Texture,
                             GLState,
                             gen::CameraSelect,
                             gen::StaticCamera,
                             gen::StaticOrthoCamera,
                             StaticGLState,
                             StaticVBOMan>
{
public:

  static const char* getName() {return "ren:RenderSimpleGeomSys";}

  bool isComponentOptional(uint64_t type) override
  {
    return es::OptionalComponents<Texture,
                                  GLState,
                                  StaticGLState,
                                  CommonUniforms,
                                  gen::CameraSelect,
                                  VecUniform,
                                  MatUniform>(type);
  }

  void groupExecute(
      es::ESCoreBase&, uint64_t /* entityID */,
      const es::ComponentGroup<gen::Transform>& trafo,
      const es::ComponentGroup<gen::StaticGlobalTime>& time,
      const es::ComponentGroup<VBO>& vbo,
      const es::ComponentGroup<IBO>& ibo,
      const es::ComponentGroup<RenderSimpleGeom>& geom,
      const es::ComponentGroup<CommonUniforms>& commonUniforms,
      const es::ComponentGroup<VecUniform>& vecUniforms,
      const es::ComponentGroup<MatUniform>& matUniforms,
      const es::ComponentGroup<Shader>& shader,
      const es::ComponentGroup<Texture>& textures,
      const es::ComponentGroup<GLState>& state,
      const es::ComponentGroup<gen::CameraSelect>& camSelect,
      const es::ComponentGroup<gen::StaticCamera>& camera,
      const es::ComponentGroup<gen::StaticOrthoCamera>& orthoCamera,
      const es::ComponentGroup<StaticGLState>& defaultGLState,
      const es::ComponentGroup<StaticVBOMan>& vboMan) override
  {
    // Setup *everything*. We don't want to enter multiple conditional
    // statements if we can avoid it. So we assume everything has not been
    // setup (including uniforms) if the simple geom hasn't been setup.
    if (geom.front().isSetUp() == false)
    {
      // We use const cast to get around a 'modify' call for 2 reasons:
      // 1) This is populating system specific GL data. It has no bearing on the
      //    actual simulation state.
      // 2) It is more correct than issuing a modify call. The data is used
      //    directly below to render geometry.
      const_cast<RenderSimpleGeom&>(geom.front()).checkAttribArray(
          vbo.front().glid, shader.front().glid, vboMan.front());

      /// \todo Optimize by pulling uniforms only once.
      if (commonUniforms.size() > 0)
      {
        const_cast<CommonUniforms&>(commonUniforms.front()).checkUniformArray(
            shader.front().glid);
      }

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

    // Textures are optional, so they are not guaranteed to have their
    // promises fulfilled at the same time as the other assets. This may
    // be a reason to use a separate system for textured objects.
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
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo.front().glid));

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

    // Apply textures (if any).
    for (const Texture& tex : textures) tex.applyUniform();

    // Apply vector uniforms (if any).
    for (const VecUniform& unif : vecUniforms) unif.applyUniform();

    // Apply matrix uniforms (if any).
    for (const MatUniform& unif : matUniforms) unif.applyUniform();

    shaders::bindPreappliedAttrib(geom.front().appliedAttribs,
                                  static_cast<size_t>(geom.front().attribSize),
                                  geom.front().stride);

    GL(glDrawElements(ibo.front().primMode, ibo.front().numPrims,
                      ibo.front().primType, 0));

    shaders::unbindPreappliedAttrib(geom.front().appliedAttribs, 
                                    static_cast<size_t>(geom.front().attribSize));

    // Reapply the default state here -- only do this if static state is
    // present.
    if (state.size() > 0 && defaultGLState.size() > 0)
    {
      defaultGLState.front().state.applyRelative(state.front().state);
    }
  }
};

void registerSystem_RenderSimpleGeom(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<RenderSimpleGeomSys>();
}

const char* getSystemName_RenderSimpleGeom()
{
  return RenderSimpleGeomSys::getName();
}

} // namespace ren

