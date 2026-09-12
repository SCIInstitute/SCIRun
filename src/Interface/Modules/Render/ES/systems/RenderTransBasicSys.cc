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
#include <es-render/comp/StaticIBOMan.hpp>
#include <es-render/comp/StaticTextureMan.hpp>

#include "RenderTransCommon.h"

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
    const spire::ComponentGroup<ren::StaticIBOMan>&)
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
    char* sbuffer = !sorted_buffer.empty() ? reinterpret_cast<char*>(&sorted_buffer[0]) : nullptr;
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
      const spire::ComponentGroup<ren::StaticTextureMan>&) override
  {
    /// \todo This needs to be moved to pre-execute.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      return;
    }

    if (srstate.front().state.get(RenderState::ActionFlags::IS_TEXT))
    {
      return;
    }

    bool doRender = srstate.front().state.get(RenderState::ActionFlags::USE_TRANSPARENCY) ||
      srstate.front().state.get(RenderState::ActionFlags::USE_TRANSPARENT_EDGES) ||
      srstate.front().state.get(RenderState::ActionFlags::USE_TRANSPARENT_NODES);

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

    renderTransparentGeometry(geom, srstate, rlist, lightUniforms,
        clippingPlaneUniforms, trafo, time, vbo, ibo, textures, commonUniforms,
        vecUniforms, matUniforms, shader, state, worldLight, clippingPlanes,
        camera, defaultGLState, vboMan, iboID,
        [this, iboID, drawLines, &pass]()
        {
          if (!drawLines &&
              pass.front().renderState.mSortType ==
                  RenderState::TransparencySortType::CONTINUOUS_SORT)
          {
            removeIBO(iboID);
          }
        });
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
