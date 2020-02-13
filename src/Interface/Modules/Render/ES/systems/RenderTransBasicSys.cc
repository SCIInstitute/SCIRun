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
#include <es-render/comp/StaticIBOMan.hpp>
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
using namespace SCIRun::Graphics::Datatypes;

// Every component is self contained. It only accesses the systems and
// components that it specifies in it's component list.

namespace SCIRun {
namespace Render {

class RenderBasicSysTrans :
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
                             SpireSubPass,
                             StaticWorldLight,
                             StaticClippingPlanes,
                             gen::StaticCamera,
                             ren::StaticGLState,
                             ren::StaticVBOMan,
                             ren::StaticIBOMan,
                             ren::StaticTextureMan>
{
public:

  static const char* getName() {return "RenderTransBasicSys";}

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

private:
  class SortedObject
  {
  public:
    std::string mName;
    GLuint mSortedID;
    Core::Geometry::Vector prevDir = Core::Geometry::Vector(0.0);

    SortedObject() :
      mSortedID(0)
    {}

    SortedObject(const std::string& name, GLuint ID, Core::Geometry::Vector& dir) :
      mName(name),
      mSortedID(ID),
      prevDir(dir)
    {}
  };

  std::vector<SortedObject> sortedObjects;

  class DepthIndex {
  public:
    size_t mIndex;
    double mDepth;

    DepthIndex() :
      mIndex(0),
      mDepth(0.0)
    {}

    DepthIndex(size_t index, double depth) :
      mIndex(index),
      mDepth(depth)
    {}

    bool operator<(const DepthIndex& di) const
    {
      return this->mDepth < di.mDepth;
    }
  };

  GLuint addIBO(void* iboData, size_t iboDataSize)
  {
    GLuint glid;

    GL(glGenBuffers(1, &glid));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glid));
    GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(iboDataSize), iboData, GL_STATIC_DRAW));

    return glid;
  }

  void removeIBO(GLuint glid)
  {
    GL(glDeleteBuffers(1, &glid));
  }

  GLuint sortObjects(const Core::Geometry::Vector& dir,
    const spire::ComponentGroup<ren::IBO>& ibo,
    const spire::ComponentGroup<SpireSubPass>& pass,
    const spire::ComponentGroup<ren::StaticIBOMan>& iboMan)
  {
    char* vbo_buffer = reinterpret_cast<char*>(pass.front().vbo.data->getBuffer());
    uint32_t* ibo_buffer = reinterpret_cast<uint32_t*>(pass.front().ibo.data->getBuffer());
    size_t num_triangles = pass.front().ibo.data->getBufferSize() / (sizeof(uint32_t) * 3);

    size_t stride_vbo = 0;
    for (auto a : pass.front().vbo.attributes)
      stride_vbo += a.sizeInBytes;

    std::vector<DepthIndex> rel_depth(num_triangles);


    for (size_t j = 0; j < num_triangles; j++)
    {
      float* vertex1 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3]));
      Core::Geometry::Point node1(vertex1[0], vertex1[1], vertex1[2]);

      float* vertex2 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3 + 1]));
      Core::Geometry::Point node2(vertex2[0], vertex2[1], vertex2[2]);

      float* vertex3 = reinterpret_cast<float*>(vbo_buffer + stride_vbo * (ibo_buffer[j * 3 + 2]));
      Core::Geometry::Point node3(vertex3[0], vertex3[1], vertex3[2]);

      rel_depth[j].mDepth = Core::Geometry::Dot(dir, node1) + Core::Geometry::Dot(dir, node2) + Core::Geometry::Dot(dir, node3);
      rel_depth[j].mIndex = j;
    }

    std::sort(rel_depth.begin(), rel_depth.end());

    std::vector<char> sorted_buffer(pass.front().ibo.data->getBufferSize());
    char* ibuffer = reinterpret_cast<char*>(pass.front().ibo.data->getBuffer());
    char* sbuffer = !sorted_buffer.empty() ? reinterpret_cast<char*>(&sorted_buffer[0]) : 0;
    GLuint result = ibo.front().glid;
    if (sbuffer && num_triangles > 0)
    {
      size_t tri_size = pass.front().ibo.data->getBufferSize() / num_triangles;

      for (size_t j = 0; j < num_triangles; j++)
      {
        memcpy(sbuffer + j * tri_size, ibuffer + rel_depth[j].mIndex * tri_size, tri_size);
      }

      std::string transIBOName = pass.front().ibo.name + "trans";
      result = addIBO(sbuffer, pass.front().ibo.data->getBufferSize());
    }

    return result;
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
      const spire::ComponentGroup<SpireSubPass>& pass,
      const spire::ComponentGroup<StaticWorldLight>& worldLight,
      const spire::ComponentGroup<StaticClippingPlanes>& clippingPlanes,
      const spire::ComponentGroup<gen::StaticCamera>& camera,
      const spire::ComponentGroup<ren::StaticGLState>& defaultGLState,
      const spire::ComponentGroup<ren::StaticVBOMan>& vboMan,
      const spire::ComponentGroup<ren::StaticIBOMan>& iboMan,
      const spire::ComponentGroup<ren::StaticTextureMan>& texMan) override
  {
    /// \todo This needs to be moved to pre-execute.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      return;
    }

    if (srstate.front().state.get(RenderState::IS_TEXT))
    {
      return;
    }

    bool doRender = srstate.front().state.get(RenderState::USE_TRANSPARENCY) ||
      srstate.front().state.get(RenderState::USE_TRANSPARENT_EDGES) ||
      srstate.front().state.get(RenderState::USE_TRANSPARENT_NODES);

    if (!doRender)
    {
      return;
    }

    bool drawLines = (ibo.front().primMode == static_cast<int>(SpireIBO::PRIMITIVE::LINES));
    GLuint iboID = ibo.front().glid;

    Core::Geometry::Vector dir(camera.front().data.view[0][2],
                               camera.front().data.view[1][2],
                               camera.front().data.view[2][2]);

    if (!drawLines)
    {
      switch (pass.front().renderState.mSortType)
      {
        case RenderState::TransparencySortType::CONTINUOUS_SORT:
        {
          iboID = sortObjects(dir, ibo, pass, iboMan);
          break;
        }
        case RenderState::TransparencySortType::UPDATE_SORT:
        {
          unsigned int index = 0;
          bool indexed = false;
          for (int i = 0; i < sortedObjects.size(); ++i)
          {
            if (sortedObjects[i].mName == pass.front().ibo.name)
            {
              indexed = true;
              index = i;
            }
          }
          if (!indexed)
          {
            index = sortedObjects.size();
            sortedObjects.push_back(SortedObject(pass.front().ibo.name, 0, dir));
          }

          Core::Geometry::Vector diff = sortedObjects[index].prevDir - dir;
          double distance = sqrtf(Dot(diff, diff));
          if (distance >= 1.23 || sortedObjects[index].mSortedID == 0)
          {
            if (sortedObjects[index].mSortedID != 0)
            {
              removeIBO(sortedObjects[index].mSortedID);
            }
            sortedObjects[index].prevDir = dir;
            sortedObjects[index].mSortedID = sortObjects(dir, ibo, pass, iboMan);
          }
          iboID = sortedObjects[index].mSortedID;
          break;
        }
        case RenderState::TransparencySortType::LISTS_SORT:
        {
          GLuint iboXID = ibo.front().glid;
          GLuint iboYID = ibo.front().glid;
          GLuint iboZID = ibo.front().glid;
          GLuint iboNegXID = ibo.front().glid;
          GLuint iboNegYID = ibo.front().glid;
          GLuint iboNegZID = ibo.front().glid;

          int index = 0;
          for (auto it = ibo.begin(); it != ibo.end(); ++it, ++index)
          {
            if (index == 1)
              iboXID = it->glid;
            if (index == 2)
              iboYID = it->glid;
            if (index == 3)
              iboZID = it->glid;
            if (index == 4)
              iboNegXID = it->glid;
            if (index == 5)
              iboNegYID = it->glid;
            if (index == 6)
              iboNegZID = it->glid;
          }

          Core::Geometry::Vector absDir(fabs(camera.front().data.view[0][2]),
                                        fabs(camera.front().data.view[1][2]),
                                        fabs(camera.front().data.view[2][2]));

          double xORy = absDir.x() > absDir.y() ? absDir.x() : absDir.y();
          double orZ = absDir.z() > xORy ? absDir.z() : xORy;

          if (orZ == absDir.x())
          {
            iboID = dir.x() < orZ ? iboNegXID : iboXID;
          }
          if (orZ == absDir.y())
          {
            iboID = dir.y() < orZ ? iboNegYID : iboYID;
          }
          if (orZ == absDir.z())
          {
            iboID = dir.z() < orZ ? iboNegZID : iboZID;
          }
          break;
        }
      }
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
    //if (srstate.front().state.get(RenderState::USE_TRANSPARENCY))
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


    if (!drawLines)
    {
      if (pass.front().renderState.mSortType == RenderState::TransparencySortType::CONTINUOUS_SORT)
      {
        removeIBO(iboID);
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

void registerSystem_RenderBasicTransGeom(spire::Acorn& core)
{
	core.registerSystem<RenderBasicSysTrans>();
}

const char* getSystemName_RenderBasicTransGeom()
{
	return RenderBasicSysTrans::getName();
}

} // namespace Render
} // namespace SCIRun
