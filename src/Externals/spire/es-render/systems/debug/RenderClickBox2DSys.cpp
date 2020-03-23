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
#include <es-general/comp/ClickBox2D.hpp>

#include "../../comp/StaticShaderMan.hpp"
#include "../../comp/StaticVBOMan.hpp"
#include "../../comp/VecUniform.hpp"
#include "../../comp/CommonUniforms.hpp"
#include "../../comp/GLState.hpp"
#include "../../comp/StaticGLState.hpp"
#include "../../comp/RenderSimpleGeom.hpp"
#include "../../util/Lines.hpp"

namespace es = spire;
namespace shaders = spire;

namespace ren {

// This debug render will be surprisingly quick because the same VBO, IBO,
// and Shader are used across all components.
class DebugRenderClickBox2DSys :
    public spire::GenericSystem<true,
                             gen::Transform,
                             gen::ClickBox2D,
                             gen::StaticGlobalTime,
                             gen::StaticOrthoCamera>
{
public:

  static const char* getName() {return "ren:RenderZZZClickBoxSys";}

  ren::VBO mVBO;
  ren::IBO mIBO;

  RenderSimpleGeom  mAttribs;
  CommonUniforms    mCommonUniforms;

  GLuint mShader;

  bool isComponentOptional(uint64_t type) override
  {
    return spire::OptionalComponents<gen::Transform>(type);
  }

  void preWalkComponents(spire::ESCoreBase& coreIn) override
  {
    auto ourCorePtr = dynamic_cast<spire::CerealCore*>(&coreIn);
    if (!ourCorePtr)
    {
      std::cerr << "Unable to execute clickbox promise fulfillment. Bad cast." << std::endl;
      return;
    }
    spire::CerealCore& core = *ourCorePtr;


    ren::StaticVBOMan& vboMan = *core.getStaticComponent<ren::StaticVBOMan>();

    auto vboIbo = getLineUnitSquare(core);
    mVBO = vboIbo.first;
    mIBO = vboIbo.second;

    mShader = getColorLineShader(core);

    // Use render simple geom as our work horse here.
    mAttribs.setAsUninitialized();
    mAttribs.checkAttribArray(mVBO.glid, mShader, vboMan);

    // Bind shader.
    GL(glUseProgram(mShader));

    // Bind VBO and IBO
    GL(glBindBuffer(GL_ARRAY_BUFFER, mVBO.glid));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO.glid));

    // Setup common uniforms.
    mCommonUniforms.checkUniformArray(mShader);

    // Setup color uniform.
    VecUniform colorUniform("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), VecUniform::VEC4);
    colorUniform.checkUniform(mShader);
    colorUniform.applyUniform();

    shaders::bindPreappliedAttrib(mAttribs.appliedAttribs,
                                  static_cast<size_t>(mAttribs.attribSize),
                                  mAttribs.stride);
  }

  void postWalkComponents(spire::ESCoreBase& core) override
  {
    shaders::unbindPreappliedAttrib(mAttribs.appliedAttribs,
                                    static_cast<size_t>(mAttribs.attribSize));
  }

  void groupExecute(
      spire::ESCoreBase&, uint64_t /* entityID */,
      const spire::ComponentGroup<gen::Transform>& trafo,
      const spire::ComponentGroup<gen::ClickBox2D>& clickBox,
      const spire::ComponentGroup<gen::StaticGlobalTime>& time,
      const spire::ComponentGroup<gen::StaticOrthoCamera>& camera) override
  {
    for (const gen::ClickBox2D& box : clickBox)
    {
      // Bind any common uniforms.
      glm::mat4 xformToUse;
      if (trafo.size() > 0)
        xformToUse = trafo.front().transform;

      glm::vec2 boxCenter = box.hitBox.getCenter();
      glm::vec2 boxExtents = box.hitBox.getExtents();
      xformToUse[0][0] *= boxExtents.x;
      xformToUse[1][1] *= boxExtents.y;

      xformToUse[3][0] += boxCenter.x;
      xformToUse[3][1] += boxCenter.y;

      mCommonUniforms.applyCommonUniforms(
          xformToUse, camera.front().data, time.front().globalTime);

      GL(glDrawElements(mIBO.primMode, mIBO.numPrims,
                        mIBO.primType, 0));
    }
  }
};

void registerSystem_DebugRenderClickBox2D(spire::Acorn& core)
{
  core.registerSystem<DebugRenderClickBox2DSys>();
}

const char* getSystemName_DebugRenderClickBox2D()
{
  return DebugRenderClickBox2DSys::getName();
}

} // namespace ren
