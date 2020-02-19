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


#include <es-general/comp/Transform.hpp>

#include "TexQuad.hpp"
#include "es-render/TextureMan.hpp"
#include "es-render/comp/StaticVBOMan.hpp"
#include "es-render/comp/StaticIBOMan.hpp"
#include "es-render/comp/StaticTextureMan.hpp"
#include "es-render/comp/VBO.hpp"
#include "es-render/comp/IBO.hpp"

namespace ren {

void addTexQuad(spire::CerealCore& core, uint64_t entityID, const std::string& texture)
{
  // Obtain the common quad IBO and VBO from rendering system.
  auto vboIbo = getTexUnitQuad(core);
  core.addComponent(entityID, vboIbo.first);
  core.addComponent(entityID, vboIbo.second);

  std::shared_ptr<ren::TextureMan> texMan = core.getStaticComponent<ren::StaticTextureMan>()->instance_;
  texMan->loadTexture(core, entityID, texture, 0, "uTX0");
}

void setTexQuadTransform(spire::CerealCore& core, uint64_t entityID, const glm::vec3& center,
                         float width, float height)
{
  gen::Transform xform;
  xform.transform = getTexQuadTransform(center, width, height);
  core.addComponent(entityID, xform);
}

void setTexQuadTransform(spire::CerealCore& core, uint64_t entityID, const glm::vec2& topLeft,
                         const glm::vec2& bottomRight, float z)
{

  gen::Transform xform;
  xform.transform = getTexQuadTransform(topLeft, bottomRight, z);
  core.addComponent(entityID, xform);
}

std::pair<ren::VBO, ren::IBO> getTexUnitQuad(spire::ESCoreBase& core)
{
  ren::VBO vboComp;
  ren::IBO iboComp;
  std::weak_ptr<ren::VBOMan> vm = core.getStaticComponent<ren::StaticVBOMan>()->instance_;
  std::weak_ptr<ren::IBOMan> im = core.getStaticComponent<ren::StaticIBOMan>()->instance_;

  if (std::shared_ptr<ren::VBOMan> vboMan = vm.lock()) {
      if (std::shared_ptr<ren::IBOMan> iboMan = im.lock()) {
          const std::string assetName = "_g_uquad";

          GLuint vbo = vboMan->hasVBO(assetName);
          if (vbo == 0)
          {
            // Build the vertex buffer object and add it to the vbo manager.
            // Build a VBO.
            const size_t vboFloatSize = 4*5;
            float vboData[vboFloatSize] =
            {
              // Position           UV coords
              -1.0f,  1.0f,  0.0f,  0.0f, 0.0f,
               1.0f,  1.0f,  0.0f,  1.0f, 0.0f,
              -1.0f, -1.0f,  0.0f,  0.0f, 1.0f,
               1.0f, -1.0f,  0.0f,  1.0f, 1.0f
            };
            size_t vboByteSize = vboFloatSize * sizeof(float);

            vbo = vboMan->addInMemoryVBO(
                static_cast<void*>(vboData), vboByteSize,
                { std::make_tuple("aPos", 3 * sizeof(float), false),
                  std::make_tuple("aUV0", 2 * sizeof(float), false) },
                assetName);
          }

          // Build VBO component to add to the entityID.
          vboComp.glid = vbo;

          const size_t iboIntSize = 6;
          iboComp.primType = GL_UNSIGNED_SHORT;
          iboComp.primMode = GL_TRIANGLES;
          iboComp.numPrims = iboIntSize;

          GLuint ibo = iboMan->hasIBO(assetName);
          if (ibo == 0)
          {
            // Build an IBO
            uint16_t iboData[iboIntSize] =
            {
              0, 1, 2, 2, 1, 3
            };
            size_t iboByteSize = iboIntSize * sizeof(uint16_t);

            ibo = iboMan->addInMemoryIBO(static_cast<void*>(iboData), iboByteSize,
                                        iboComp.primMode, iboComp.primType,
                                        iboComp.numPrims, assetName);
          }

          iboComp.glid = ibo;
      }
  }
  return std::make_pair(vboComp, iboComp);
}

glm::mat4 getTexQuadTransform(const glm::vec3& center, float width, float height)
{
  glm::mat4 trafo;
  trafo[3] = glm::vec4(center.x, center.y, center.z, 1.0f);

  float halfWidth = width / 2.0f;
  float halfHeight = height / 2.0f;
  trafo[0][0] = halfWidth;
  trafo[1][1] = halfHeight;

  return trafo;
}

glm::mat4 getTexQuadTransform(const glm::vec2& topLeft, const glm::vec2& bottomRight, float z)
{
  glm::mat4 trafo;
  float width = bottomRight.x - topLeft.x;
  float height = topLeft.y - bottomRight.y;
  glm::vec3 center = glm::vec3(width / 2.0f, height / 2.0f, z);
  return getTexQuadTransform(center, width, height);
}

} // namespace gp
