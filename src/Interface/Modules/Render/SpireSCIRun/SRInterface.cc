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
    mMouseMode(MOUSE_OLDSCIRUN),
    mScreenWidth(640),
    mScreenHeight(480),
    mCamera(new SRCamera(*this, mSpire))                       // Should come after all vars have been initialized.
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

  // Load scirun5 arrow asset.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  std::fstream arrowFile("Assets/UnitArrow.sp");
  spire::Interface::loadProprietarySR5AssetFile(arrowFile, *rawVBO, *rawIBO);

  std::vector<std::string> attribNames = {"aPos", "aNormal"};
  spire::Interface::IBO_TYPE iboType = spire::Interface::IBO_16BIT;

  mArrowVBOName = "arrowVBO";
  mArrowIBOName = "arrowIBO";
  mSpire->addVBO(mArrowVBOName, rawVBO, attribNames);
  mSpire->addIBO(mArrowIBOName, rawIBO, iboType);

  mArrowObjectName = "arrowObject";
  mSpire->addObject(mArrowObjectName);
  mSpire->addPassToObject(mArrowObjectName, "DirPhong", mArrowVBOName, mArrowIBOName, spire::Interface::TRIANGLES);
}

//------------------------------------------------------------------------------
SRInterface::~SRInterface()
{
  mSpire->terminate();
}

//------------------------------------------------------------------------------
void SRInterface::setMouseMode(MouseMode mode)
{
  mMouseMode = mode;
}

//------------------------------------------------------------------------------
SRInterface::MouseMode SRInterface::getMouseMode()
{
  return mMouseMode;
}

//------------------------------------------------------------------------------
void SRInterface::eventResize(size_t width, size_t height)
{
  mScreenWidth = width;
  mScreenHeight = height; 

  mSpire->makeCurrent();
  GL(glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height)));

  /// \todo Ensure perspective matrix is rebuilt with correct aspect ratio.
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseDown(const glm::ivec2& pos, MouseButton btn)
{
  mCamera->mouseDownEvent(pos, btn);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseMove(const glm::ivec2& pos, MouseButton btn)
{
  mCamera->mouseMoveEvent(pos, btn);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseWheel(int32_t delta)
{
  mCamera->mouseWheelEvent(delta);
}

//------------------------------------------------------------------------------
void SRInterface::doAutoView()
{
  if (mSceneBBox.valid())
    mCamera->doAutoView(mSceneBBox);
}

//------------------------------------------------------------------------------
void SRInterface::inputMouseUp(const glm::ivec2& /*pos*/, MouseButton /*btn*/)
{
}

//------------------------------------------------------------------------------
void SRInterface::handleGeomObject(boost::shared_ptr<Core::Datatypes::GeometryObject> obj)
{
  // Ensure our rendering context is current on our thread.
  mSpire->makeCurrent();

  std::string objectName = obj->objectName;
  Core::Geometry::BBox bbox; // Bounding box containing all vertex buffer objects.

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
    bbox.extend(it->boundingBox);
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
  mSRObjects.push_back(SRObject(objectName, xform, bbox));
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
      std::shared_ptr< ::spire::AbstractUniformStateItem > uniform(std::get<1>(*it));

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

  // Ensure we have an appropriate camera transform.
  mCamera->applyTransform();
  glm::mat4 viewToWorld = mCamera->getViewToWorld();

  mSceneBBox.reset();

  // Set directional light source (in world space).
  glm::vec3 viewDir = viewToWorld[2].xyz();
  viewDir = -viewDir; // Cameras look down -Z.
  mSpire->addGlobalUniform("uLightDirWorld", viewDir);
  //mSpire->addGlobalUniform("uLightDirWorld", glm::vec3(1.0f, 0.0f, 0.0f));

  for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
  {
    if (it->mBBox.valid())
      mSceneBBox.extend(it->mBBox);

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

  // Now render the axes on the screen.
  float aspect = static_cast<float>(640) / static_cast<float>(480);
  glm::mat4 projection = glm::perspective(0.59f, aspect, 1.0f, 2000.0f);

  // Build world transform for all axes. Rotates about uninverted camera's
  // view, then translates to a specified corner on the screen.
  glm::mat4 axesRot = mCamera->getWorldToView();
  axesRot[3][0] = 0.0f;
  axesRot[3][1] = 0.0f;
  axesRot[3][2] = 0.0f;
  glm::mat4 invCamTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0.42f, 0.39f, -1.5f));
  glm::mat4 axesScale = glm::scale(glm::mat4(1.0f), glm::vec3(0.05));;
  glm::mat4 axesTransform = axesScale * axesRot;

  mSpire->addObjectPassUniform(mArrowObjectName, "uCamViewVec", glm::vec3(0.0f, 0.0f, -1.0f));
  mSpire->addObjectPassUniform(mArrowObjectName, "uLightDirWorld", glm::vec3(0.0f, 0.0f, -1.0f));

  // Build projection for the axes to use on the screen. The arrors will not
  // use the camera, but will use the camera's transformation matrix.

  // X Axis
  {
    glm::mat4 xform = glm::rotate(glm::mat4(1.0f), spire::PI / 2.0f, glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 finalTrafo = axesTransform * xform;

    mSpire->addObjectPassUniform(mArrowObjectName, "uAmbientColor", glm::vec4(0.5f, 0.01f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uDiffuseColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uSpecularColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uSpecularPower", 16.0f);

    // Add appropriate projection and object -> world transformations.
    // Light will always be directed down the camera's axis.
    mSpire->addObjectPassUniform(mArrowObjectName, "uProjIVObject", projection * invCamTrans * finalTrafo);
    mSpire->addObjectPassUniform(mArrowObjectName, "uObject", finalTrafo);

    mSpire->renderObject(mArrowObjectName);
  }

  // Y Axis
  {
    glm::mat4 xform = glm::rotate(glm::mat4(1.0f), -spire::PI / 2.0f, glm::vec3(1.0, 0.0, 0.0));
    glm::mat4 finalTrafo = axesTransform * xform;

    mSpire->addObjectPassUniform(mArrowObjectName, "uAmbientColor", glm::vec4(0.01f, 0.5f, 0.01f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uDiffuseColor", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uSpecularColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uSpecularPower", 16.0f);


    // Add appropriate projection and object -> world transformations.
    // Light will always be directed down the camera's axis.
    mSpire->addObjectPassUniform(mArrowObjectName, "uProjIVObject", projection * invCamTrans * finalTrafo);
    mSpire->addObjectPassUniform(mArrowObjectName, "uObject", finalTrafo);

    mSpire->renderObject(mArrowObjectName);
  }

  // Z Axis
  {
    // No rotation at all
    glm::mat4 finalTrafo = axesTransform;

    mSpire->addObjectPassUniform(mArrowObjectName, "uAmbientColor", glm::vec4(0.01f, 0.01f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uDiffuseColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uSpecularColor", glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    mSpire->addObjectPassUniform(mArrowObjectName, "uSpecularPower", 16.0f);


    // Add appropriate projection and object -> world transformations.
    // Light will always be directed down the camera's axis.
    mSpire->addObjectPassUniform(mArrowObjectName, "uProjIVObject", projection * invCamTrans * finalTrafo);
    mSpire->addObjectPassUniform(mArrowObjectName, "uObject", finalTrafo);

    mSpire->renderObject(mArrowObjectName);
  }

}



} // namespace Gui
} // namespace SCIRun 

