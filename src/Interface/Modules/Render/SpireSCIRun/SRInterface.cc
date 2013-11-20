/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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

/// \author James Hughes
/// \date   February 2013

#include "../namespaces.h"
#include "SRInterface.h"
#include "SciBall.h"
#include "SRCamera.h"

#include "spire/src/Hub.h"
#include "spire/src/InterfaceImplementation.h"

#include "SRCommonAttributes.h"
#include "SRCommonUniforms.h"

using namespace std::placeholders;

namespace SCIRun {
namespace Gui {


//------------------------------------------------------------------------------
SRInterface::SRInterface(std::shared_ptr<spire::Context> context,
                         const std::vector<std::string>& shaderDirs,
                         spire::Interface::LogFunction logFP) :
    mSpire(new spire::Interface(context, shaderDirs, logFP)),
    mCamDistance(7.0f),
    mScreenWidth(640),
    mScreenHeight(480),
    mCamAccumPosDown(0.0f, 0.0f, 0.0f),
    mCamAccumPosNow(0.0f, 0.0f, 0.0f),
    mCamera(new SRCamera(*this, mSpire)),                       // Should come after all vars have been initialized.
    mSciBall(new SciBall(spire::V3(0.0f, 0.0f, 0.0f), 1.0f))   // Should come after all vars have been initialized.
{

  // Add shader attributes that we will be using.
  mSpire->addShaderAttribute("aPos",         3,  false,  sizeof(float) * 3,  spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aNormal",      3,  false,  sizeof(float) * 3,  spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aFieldData",   1,  false,  sizeof(float),      spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aColorFloat",  4,  false,  sizeof(float) * 4,  spire::Interface::TYPE_FLOAT);
  mSpire->addShaderAttribute("aColor",       4,  true,   sizeof(char) * 4,   spire::Interface::TYPE_UBYTE);

  std::vector<std::tuple<std::string, spire::Interface::SHADER_TYPES>> shaderFiles;
  shaderFiles.push_back(std::make_pair("UniformColor.vsh", spire::Interface::VERTEX_SHADER));
  shaderFiles.push_back(std::make_pair("UniformColor.fsh", spire::Interface::FRAGMENT_SHADER));
  mSpire->addPersistentShader("UniformColor", shaderFiles);

  shaderFiles.clear();
  shaderFiles.push_back(std::make_pair("DirPhong.vsh", spire::Interface::VERTEX_SHADER));
  shaderFiles.push_back(std::make_pair("DirPhong.fsh", spire::Interface::FRAGMENT_SHADER));
  mSpire->addPersistentShader("DirPhong", shaderFiles);

  buildAndApplyCameraTransform();
}

//------------------------------------------------------------------------------
SRInterface::~SRInterface()
{
  mSpire->terminate();
}

//------------------------------------------------------------------------------
void SRInterface::eventResize(size_t width, size_t height)
{
  mScreenWidth = width;
  mScreenHeight = height; 

  mSpire->makeCurrent();
  GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));
}

//------------------------------------------------------------------------------
spire::V2 SRInterface::calculateScreenSpaceCoords(const glm::ivec2& mousePos)
{
  float windowOriginX = 0.0f;
  float windowOriginY = 0.0f;

  // Transform incoming mouse coordinates into screen space.
  spire::V2 mouseScreenSpace;
  mouseScreenSpace.x = 2.0f * (static_cast<float>(mousePos.x) - windowOriginX) 
      / static_cast<float>(mScreenWidth) - 1.0f;
  mouseScreenSpace.y = 2.0f * (static_cast<float>(mousePos.y) - windowOriginY)
      / static_cast<float>(mScreenHeight) - 1.0f;

  // Rotation with flipped axes feels much more natural.
  mouseScreenSpace.y = -mouseScreenSpace.y;

  return mouseScreenSpace;
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseDown(const glm::ivec2& pos, MouseButton btn)
{
  // Translation variables.
  mCamAccumPosDown  = mCamAccumPosNow;
  mTransClick       = calculateScreenSpaceCoords(pos);

  if (btn == MOUSE_LEFT)
  {
    spire::V2 mouseScreenSpace = calculateScreenSpaceCoords(pos);
    mSciBall->beginDrag(mouseScreenSpace);
  }
  else if (btn == MOUSE_RIGHT)
  {
    // Store translation starting position.
  }
  mActiveDrag = btn;
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseMove(const glm::ivec2& pos, MouseButton btn)
{
  if (mActiveDrag == btn)
  {
    if (btn == MOUSE_LEFT)
    {
      spire::V2 mouseScreenSpace = calculateScreenSpaceCoords(pos);
      mSciBall->drag(mouseScreenSpace);

      buildAndApplyCameraTransform();
    }
    else if (btn == MOUSE_RIGHT)
    {
      spire::V2 curTrans = calculateScreenSpaceCoords(pos);
      spire::V2 delta = curTrans - mTransClick;
      /// \todo This 2.5f value is a magic number, and it's real value should
      ///       be calculated based off of the world space position of the
      ///       camera. This value could easily be calculated based off of
      ///       mCamDistance.
      spire::V2 trans = (-delta) * 2.5f;

      spire::M44 camRot = mSciBall->getTransformation();
      spire::V3 translation =   static_cast<spire::V3>(camRot[0].xyz()) * trans.x
                       + static_cast<spire::V3>(camRot[1].xyz()) * trans.y;
      mCamAccumPosNow = mCamAccumPosDown + translation;

      buildAndApplyCameraTransform();
    }
  }
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseWheel(int32_t delta)
{
  // Reason why we subtract: Feels more natural to me =/.
  mCamDistance -= static_cast<float>(delta) / 100.0f;
  buildAndApplyCameraTransform();
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseUp(const glm::ivec2& /*pos*/, MouseButton /*btn*/)
{
}

//------------------------------------------------------------------------------
void SRInterface::buildAndApplyCameraTransform()
{
  spire::M44 camRot      = mSciBall->getTransformation();
  spire::M44 finalTrafo  = camRot;

  // Translation is a post rotation operation where as zoom is a pre transform
  // operation. We should probably ensure the user doesn't scroll passed zero.
  // Remember, we are looking down NEGATIVE z.
  finalTrafo[3].xyz() = mCamAccumPosNow + static_cast<spire::V3>(camRot[2].xyz()) * mCamDistance;

  mCamera->setViewTransform(finalTrafo);
}


//------------------------------------------------------------------------------
void SRInterface::handleGeomObject(boost::shared_ptr<Core::Datatypes::GeometryObject> obj)
{
  // Ensure our rendering context is current on our thread.
  mSpire->makeCurrent();

  std::string objectName = obj->objectName;

  // Check to see if the object already exists in our list. If so, then
  // remove the object. We will re-add it.
  auto foundObject = std::find_if(
      mSRObjects.begin(), mSRObjects.end(),
      [&objectName, this](const SRObject& obj) -> bool
      {
        if (obj.mName == objectName)
          return true;
        else
          return false;
      });

  if (foundObject != mSRObjects.end())
  {
    // Remove the object from spire.
    mSpire->removeObject(objectName);
    mSRObjects.erase(foundObject);
  }

  // Now we re-add the object to spire.
  mSpire->addObject(objectName);

  // Add vertex buffer objects.
  for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireVBO& vbo = *it;
    mSpire->addVBO(vbo.name, vbo.data, vbo.attributeNames);
  }

  // Add index buffer objects.
  for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireIBO& ibo = *it;
    ::spire::Interface::IBO_TYPE type;
    switch (ibo.indexSize)
    {
      case 1: // 8-bit
        type = ::spire::Interface::IBO_8BIT;
        break;

      case 2: // 16-bit
        type = ::spire::Interface::IBO_16BIT;
        break;

      case 4: // 32-bit
        type = ::spire::Interface::IBO_32BIT;
        break;

      default:
        type = ::spire::Interface::IBO_32BIT;
        throw std::invalid_argument("Unable to determine index buffer depth.");
        break;
    }
    mSpire->addIBO(ibo.name, ibo.data, type);
  }

  // Add default identity transform to the object globally (instead of
  // per-pass).
  spire::M44 xform;
  xform[3] = ::spire::V4(0.0f, 0.0f, 0.0f, 1.0f);
  // Use emplace back when we switch to VS 2013.
  //mSRObjects.emplace_back(objectName, xform);
  mSRObjects.push_back(SRObject(objectName, xform));
  SRObject& elem = mSRObjects.back();

  // Add passes
  for (auto it = obj->mPasses.cbegin(); it != obj->mPasses.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireSubPass& pass = *it;
    mSpire->addPassToObject(obj->objectName, pass.programName,
                           pass.vboName, pass.iboName, pass.type,
                           pass.passName, SPIRE_DEFAULT_PASS);

    // Add uniforms associated with the pass
    for (auto it = pass.uniforms.begin(); it != pass.uniforms.end(); ++it)
    {
      std::string uniformName = std::get<0>(*it);
      std::shared_ptr<::spire::AbstractUniformStateItem> uniform(std::get<1>(*it));

      // Be sure to always include the pass name as we are updating a
      // subpass of SPIRE_DEFAULT_PASS.
      mSpire->addObjectPassUniformConcrete(obj->objectName, uniformName, 
                                          uniform, pass.passName);
    }

    // Add a pass to our local object.
    elem.mPasses.emplace_back(pass.passName);
    SRObject::SRPass& thisPass = elem.mPasses.back();

    std::vector<spire::Interface::UnsatisfiedUniform> unsatisfied;
    unsatisfied = mSpire->getUnsatisfiedUniforms(objectName, pass.passName);
    for (auto it = unsatisfied.begin(); it != unsatisfied.end(); ++it)
    {
      if (it->uniformName == SRCommonUniforms::getObjectName())
        thisPass.transforms.push_back(SRObject::OBJECT_TO_WORLD);
      else if (it->uniformName == SRCommonUniforms::getObjectToViewName())
        thisPass.transforms.push_back(SRObject::OBJECT_TO_CAMERA);
      else if (it->uniformName == SRCommonUniforms::getObjectToCameraToProjectionName())
        thisPass.transforms.push_back(SRObject::OBJECT_TO_CAMERA_PROJECTION);
    }

    // Add gpu state if it has been set.
    if (pass.hasGPUState == true)
    {
      // Be sure to always include the pass name as we are updating a
      // subpass of SPIRE_DEFAULT_PASS.
      mSpire->addObjectPassGPUState(obj->objectName, pass.gpuState, pass.passName);
    }
  }

  // Now retrieve the currently unsatisfied uniforms from the object and
  // ensure that we build the appropriate transforms for the object
  // (only when the object moves).

  // Now that we have created all of the appropriate passes, get rid of the
  // VBOs and IBOs.
  for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireVBO& vbo = *it;
    mSpire->removeVBO(vbo.name);
  }

  for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireIBO& ibo = *it;
    mSpire->removeIBO(ibo.name);
  }
}


//------------------------------------------------------------------------------
void SRInterface::removeAllGeomObjects()
{
  mSpire->makeCurrent();

  for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
  {
    mSpire->removeObject(it->mName);
  }
  mSRObjects.clear();
}

//------------------------------------------------------------------------------
void SRInterface::beginFrame()
{
  /// \todo Move this outside of the interface!
  GL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
  GL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

  /// \todo Make line width a part of the GPU state.
  glLineWidth(2.0f);
  //glEnable(GL_LINE_SMOOTH);

  spire::GPUState defaultGPUState;
  mSpire->applyGPUState(defaultGPUState, true); // true = force application of state.
}

//------------------------------------------------------------------------------
void SRInterface::doFrame()
{
  mSpire->makeCurrent();

  // Do not even attempt to render if the framebuffer is not complete.
  // This can happen when the rendering window is hidden (in SCIRun5 for
  // example);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    return;

  beginFrame();

  // Set directional light source (in world space).
  mSpire->addGlobalUniform("uLightDirWorld", spire::V3(1.0f, 0.0f, 0.0f));

  for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
  {
    spire::M44 obj = it->mObjectToWorld;
    // Setup transforms for all passes and render each of the passes.
    for (auto passit = it->mPasses.begin(); passit != it->mPasses.end(); ++passit)
    {
      for (auto trafoit = passit->transforms.begin(); trafoit != passit->transforms.end(); ++trafoit)
      {
        switch (*trafoit)
        {
          case SRObject::OBJECT_TO_WORLD:
            mSpire->addObjectPassUniform(it->mName, SRCommonUniforms::getObjectName(),
                                         obj, passit->passName);
            break;
          case SRObject::OBJECT_TO_CAMERA:
            mSpire->addObjectPassUniform(it->mName, SRCommonUniforms::getObjectToViewName(),
                                         mCamera->getWorldToView() * obj, passit->passName);
            break;
          case SRObject::OBJECT_TO_CAMERA_PROJECTION:
            mSpire->addObjectPassUniform(it->mName, SRCommonUniforms::getObjectToCameraToProjectionName(),
                                         mCamera->getWorldToProjection() * obj, passit->passName);
            break;
        }
      }
      mSpire->renderObject(it->mName, passit->passName);
    }
  }

}



} // namespace Gui
} // namespace SCIRun 

