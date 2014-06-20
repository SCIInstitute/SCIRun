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

#include "../comp/RenderColorMapGeom.h"
#include "../comp/SRRenderState.h"
#include "../comp/RenderList.h"

namespace es = CPM_ES_NS;
namespace shaders = CPM_GL_SHADERS_NS;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list.

namespace SCIRun {
namespace Render {

class RenderColorMapSys :
    public es::GenericSystem<true,
                             RenderColorMapGeom,   // TAG class
                             SRRenderState,
                             RenderList,
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
                             gen::StaticCamera,
                             ren::StaticGLState,
                             ren::StaticVBOMan>
{
public:

  static const char* getName() {return "RenderColorMapSys";}

  bool isComponentOptional(uint64_t type) override
  {
    return es::OptionalComponents<RenderList,
                                  ren::GLState,
                                  ren::StaticGLState,
                                  ren::CommonUniforms,
                                  ren::VecUniform,
                                  ren::MatUniform>(type);
  }

  void groupExecute(
      es::ESCoreBase&, uint64_t /* entityID */,
      const es::ComponentGroup<RenderColorMapGeom>& geom,
      const es::ComponentGroup<SRRenderState>& srstate,
      const es::ComponentGroup<RenderList>& rlist,
      const es::ComponentGroup<gen::Transform>& trafo,
      const es::ComponentGroup<gen::StaticGlobalTime>& time,
      const es::ComponentGroup<ren::VBO>& vbo,
      const es::ComponentGroup<ren::IBO>& ibo,
      const es::ComponentGroup<ren::Texture>& textures,
      const es::ComponentGroup<ren::CommonUniforms>& commonUniforms,
      const es::ComponentGroup<ren::VecUniform>& vecUniforms,
      const es::ComponentGroup<ren::MatUniform>& matUniforms,
      const es::ComponentGroup<ren::Shader>& shader,
      const es::ComponentGroup<ren::GLState>& state,
      const es::ComponentGroup<gen::StaticCamera>& camera,
      const es::ComponentGroup<ren::StaticGLState>& defaultGLState,
      const es::ComponentGroup<ren::StaticVBOMan>& vboMan) override
  {
    /// \todo This needs to be moved to pre-execute.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      return;
    }

    // Setup *everything*. We don't want to enter multiple conditional
    // statements if we can avoid it. So we assume everything has not been
    // setup (including uniforms) if the simple geom hasn't been setup.
    if (geom.front().attribs.isSetup() == false)
    {
      // We use const cast to get around a 'modify' call for 2 reasons:
      // 1) This is populating system specific GL data. It has no bearing on the
      //    actual simulation state.
      // 2) It is more correct than issuing a modify call. The data is used
      //    directly below to render geometry.
      const_cast<RenderColorMapGeom&>(geom.front()).attribs.setup(
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
      commonUniforms.front().applyCommonUniforms(
          trafo.front().transform, camera.front().data, time.front().globalTime);
    }

    for (const ren::Texture& tex : textures)
    {
      if (tex.isSetUp() == false)
      {
        const_cast<ren::Texture&>(tex).checkUniform(shader.front().glid);
        if (tex.isSetUp() == false)
        {
          return;
        }
      }
    }

    // Apply textures
    for (const ren::Texture& tex : textures) tex.applyUniform();

    // Apply vector uniforms (if any).
    for (const ren::VecUniform& unif : vecUniforms) {unif.applyUniform();}

    // Apply matrix uniforms (if any).
    for (const ren::MatUniform& unif : matUniforms) {unif.applyUniform();}

    geom.front().attribs.bind();

    if (srstate.front().state.get(RenderState::USE_TRANSPARENCY))
    {
      GL(glDepthMask(GL_FALSE));
      GL(glDisable(GL_CULL_FACE));
    }

    if (rlist.size() > 0)
    {
      // Render a color mapped list of VBOs/IBOs. We will be using the VBO and
      // IBO attached to this object as the basic rendering primitive.
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

    if (srstate.front().state.get(RenderState::USE_TRANSPARENCY))
    {
      GL(glDepthMask(GL_TRUE));
      GL(glEnable(GL_CULL_FACE));
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

void registerSystem_RenderColorMap(CPM_ES_ACORN_NS::Acorn& core)
{
  core.registerSystem<RenderColorMapSys>();
}

const char* getSystemName_RenderColorMap()
{
  return RenderColorMapSys::getName();
}

} // namespace Render
} // namespace SCIRun

