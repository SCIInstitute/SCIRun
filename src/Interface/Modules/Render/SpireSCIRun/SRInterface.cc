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

#include <Interface/Modules/Render/namespaces.h>
#include <Interface/Modules/Render/SpireSCIRun/SRInterface.h>
#include <Interface/Modules/Render/SpireSCIRun/SciBall.h>
#include <Interface/Modules/Render/SpireSCIRun/SRCamera.h>

#include "spire/src/Hub.h"
#include "spire/src/InterfaceImplementation.h"
#include <gl-state/GLState.hpp>

#include <Interface/Modules/Render/SpireSCIRun/SRCommonAttributes.h>
#include <Interface/Modules/Render/SpireSCIRun/SRCommonUniforms.h>

#include <Core/Application/Application.h>

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

  shaderFiles.clear();
  shaderFiles.push_back(std::make_pair("DirPhongCMap.vsh", spire::Interface::VERTEX_SHADER));
  shaderFiles.push_back(std::make_pair("DirPhongCMap.fsh", spire::Interface::FRAGMENT_SHADER));
  mSpire->addPersistentShader("DirPhongCMap", shaderFiles);

  shaderFiles.clear();
  shaderFiles.push_back(std::make_pair("ColorMap.vsh", spire::Interface::VERTEX_SHADER));
  shaderFiles.push_back(std::make_pair("ColorMap.fsh", spire::Interface::FRAGMENT_SHADER));
  mSpire->addPersistentShader("ColorMap", shaderFiles);

  // Load scirun5 arrow asset this code needs to be update with file error
  // checking in the entity system renderer.
  auto baseAssetDirQT = SCIRun::Core::Application::Instance().executablePath() / "Assets";
  std::string baseAssetDir = baseAssetDirQT.string();
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  std::ifstream arrowFile(baseAssetDir + "/UnitArrow.sp", std::ios::in | std::ios::binary);
  spire::Interface::loadProprietarySR5AssetFile(arrowFile, *rawVBO, *rawIBO);

  std::vector<std::string> attribNames;
  attribNames.push_back("aPos");
  attribNames.push_back("aNormal");
  spire::Interface::IBO_TYPE iboType = spire::Interface::IBO_16BIT;

  mArrowVBOName = "arrowVBO";
  mArrowIBOName = "arrowIBO";
  mSpire->addVBO(mArrowVBOName, rawVBO, attribNames);
  mSpire->addIBO(mArrowIBOName, rawIBO, iboType);

  mArrowObjectName = "arrowObject";
  mSpire->addObject(mArrowObjectName);
  mSpire->addPassToObject(mArrowObjectName, "DirPhong", mArrowVBOName, mArrowIBOName, spire::Interface::TRIANGLES);

  // Create default colormaps.
  generateColormaps();
}

//------------------------------------------------------------------------------
SRInterface::~SRInterface()
{
  glDeleteTextures(1, &mRainbowCMap);
  glDeleteTextures(1, &mGrayscaleCMap);
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
  mSRObjects.push_back(SRObject(objectName, xform, bbox, obj->mColorMap));
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

    //// Add gpu state if it has been set.
    //if (pass.hasGPUState == true)
    //{
    //  // Be sure to always include the pass name as we are updating a
    //  // subpass of SPIRE_DEFAULT_PASS.
    //  mSpire->addObjectPassGPUState(obj->objectName, pass.gpuState, pass.passName);
    //}
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
void SRInterface::gcInvalidObjects(const std::vector<std::string>& validObjects)
{
  for (auto it = mSRObjects.begin(); it != mSRObjects.end();)
  {
    if (std::find(validObjects.begin(), validObjects.end(), it->mName) == validObjects.end())
    {
      mSpire->removeObject(it->mName);
      it = mSRObjects.erase(it);
    }
    else
    {
      ++it;
    }
  }
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

  CPM_GL_STATE_NS::GLState defaultGLState;
  defaultGLState.setLineWidth(2.0f);
  defaultGLState.apply();
  //spire::GPUState defaultGPUState;
  //mSpire->applyGPUState(defaultGPUState, true); // true = force application of state.
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

      if (it->mColorMap && passit->passName == "facesPass")
      {
        GL(glActiveTexture(GL_TEXTURE0));
        glEnable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_3D);

        // Setup appropriate texture to render the color map.
        if (*(it->mColorMap) == "Rainbow")
        {
          GL(glBindTexture(GL_TEXTURE_1D, mRainbowCMap));
        }
        else
        {
          GL(glBindTexture(GL_TEXTURE_1D, mGrayscaleCMap));
        }

        // Need to also add uniforms for min / max color map entries.
        // Maybe we should just normalize in show field? Just for now.

        spire::SpireSampler1D_NoRAII samplerType(0);
        mSpire->addObjectPassUniform<spire::SpireSampler1D_NoRAII>(
            it->mName, "uTX0", samplerType, passit->passName);
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

float rainbowRaw[] =
{
  0.0000, 0.0000, 1.0000, 1.0,
  0.0000, 0.0216, 1.0000, 1.0,
  0.0000, 0.0432, 1.0000, 1.0,
  0.0000, 0.0648, 1.0000, 1.0,
  0.0000, 0.0864, 1.0000, 1.0,
  0.0000, 0.1080, 1.0000, 1.0,
  0.0000, 0.1296, 1.0000, 1.0,
  0.0000, 0.1511, 1.0000, 1.0,
  0.0000, 0.1727, 1.0000, 1.0,
  0.0000, 0.1943, 1.0000, 1.0,
  0.0002, 0.2104, 1.0000, 1.0,
  0.0006, 0.2220, 1.0000, 1.0,
  0.0011, 0.2336, 1.0000, 1.0,
  0.0015, 0.2453, 1.0000, 1.0,
  0.0019, 0.2569, 1.0000, 1.0,
  0.0023, 0.2685, 1.0000, 1.0,
  0.0027, 0.2801, 1.0000, 1.0,
  0.0031, 0.2918, 1.0000, 1.0,
  0.0036, 0.3034, 1.0000, 1.0,
  0.0040, 0.3149, 1.0000, 1.0,
  0.0048, 0.3253, 1.0000, 1.0,
  0.0057, 0.3356, 1.0000, 1.0,
  0.0065, 0.3460, 1.0000, 1.0,
  0.0073, 0.3564, 1.0000, 1.0,
  0.0082, 0.3668, 1.0000, 1.0,
  0.0090, 0.3772, 1.0000, 1.0,
  0.0098, 0.3875, 1.0000, 1.0,
  0.0107, 0.3979, 1.0000, 1.0,
  0.0115, 0.4083, 1.0000, 1.0,
  0.0123, 0.4192, 1.0000, 1.0,
  0.0131, 0.4304, 1.0000, 1.0,
  0.0140, 0.4417, 1.0000, 1.0,
  0.0148, 0.4529, 1.0000, 1.0,
  0.0156, 0.4641, 1.0000, 1.0,
  0.0165, 0.4753, 1.0000, 1.0,
  0.0173, 0.4865, 1.0000, 1.0,
  0.0181, 0.4977, 1.0000, 1.0,
  0.0190, 0.5089, 1.0000, 1.0,
  0.0200, 0.5200, 0.9989, 1.0,
  0.0216, 0.5303, 0.9939, 1.0,
  0.0233, 0.5407, 0.9889, 1.0,
  0.0250, 0.5511, 0.9839, 1.0,
  0.0266, 0.5615, 0.9790, 1.0,
  0.0283, 0.5719, 0.9740, 1.0,
  0.0299, 0.5822, 0.9690, 1.0,
  0.0316, 0.5926, 0.9640, 1.0,
  0.0333, 0.6030, 0.9590, 1.0,
  0.0349, 0.6134, 0.9540, 1.0,
  0.0359, 0.6221, 0.9433, 1.0,
  0.0368, 0.6304, 0.9308, 1.0,
  0.0376, 0.6388, 0.9183, 1.0,
  0.0384, 0.6471, 0.9059, 1.0,
  0.0393, 0.6554, 0.8934, 1.0,
  0.0401, 0.6637, 0.8810, 1.0,
  0.0409, 0.6720, 0.8685, 1.0,
  0.0418, 0.6803, 0.8561, 1.0,
  0.0426, 0.6886, 0.8436, 1.0,
  0.0437, 0.6963, 0.8310, 1.0,
  0.0454, 0.7030, 0.8181, 1.0,
  0.0470, 0.7096, 0.8053, 1.0,
  0.0487, 0.7163, 0.7924, 1.0,
  0.0503, 0.7229, 0.7795, 1.0,
  0.0520, 0.7296, 0.7666, 1.0,
  0.0537, 0.7362, 0.7538, 1.0,
  0.0553, 0.7428, 0.7409, 1.0,
  0.0570, 0.7495, 0.7280, 1.0,
  0.0586, 0.7561, 0.7152, 1.0,
  0.0610, 0.7631, 0.7027, 1.0,
  0.0635, 0.7702, 0.6902, 1.0,
  0.0660, 0.7773, 0.6777, 1.0,
  0.0685, 0.7843, 0.6653, 1.0,
  0.0710, 0.7914, 0.6528, 1.0,
  0.0735, 0.7984, 0.6404, 1.0,
  0.0760, 0.8055, 0.6279, 1.0,
  0.0785, 0.8125, 0.6155, 1.0,
  0.0810, 0.8196, 0.6030, 1.0,
  0.0840, 0.8263, 0.5913, 1.0,
  0.0878, 0.8325, 0.5805, 1.0,
  0.0915, 0.8388, 0.5697, 1.0,
  0.0952, 0.8450, 0.5589, 1.0,
  0.0990, 0.8512, 0.5481, 1.0,
  0.1027, 0.8574, 0.5373, 1.0,
  0.1064, 0.8637, 0.5265, 1.0,
  0.1102, 0.8699, 0.5157, 1.0,
  0.1139, 0.8761, 0.5049, 1.0,
  0.1176, 0.8824, 0.4941, 1.0,
  0.1226, 0.8873, 0.4842, 1.0,
  0.1276, 0.8923, 0.4742, 1.0,
  0.1326, 0.8973, 0.4642, 1.0,
  0.1376, 0.9023, 0.4543, 1.0,
  0.1426, 0.9073, 0.4443, 1.0,
  0.1475, 0.9122, 0.4343, 1.0,
  0.1525, 0.9172, 0.4244, 1.0,
  0.1575, 0.9222, 0.4144, 1.0,
  0.1625, 0.9272, 0.4044, 1.0,
  0.1689, 0.9319, 0.3954, 1.0,
  0.1763, 0.9365, 0.3871, 1.0,
  0.1838, 0.9411, 0.3788, 1.0,
  0.1913, 0.9457, 0.3705, 1.0,
  0.1988, 0.9502, 0.3622, 1.0,
  0.2062, 0.9548, 0.3539, 1.0,
  0.2137, 0.9594, 0.3456, 1.0,
  0.2212, 0.9639, 0.3373, 1.0,
  0.2287, 0.9685, 0.3290, 1.0,
  0.2365, 0.9729, 0.3206, 1.0,
  0.2478, 0.9758, 0.3123, 1.0,
  0.2590, 0.9787, 0.3040, 1.0,
  0.2702, 0.9816, 0.2957, 1.0,
  0.2814, 0.9845, 0.2874, 1.0,
  0.2926, 0.9874, 0.2791, 1.0,
  0.3038, 0.9903, 0.2708, 1.0,
  0.3150, 0.9932, 0.2625, 1.0,
  0.3262, 0.9961, 0.2542, 1.0,
  0.3374, 0.9990, 0.2459, 1.0,
  0.3492, 1.0000, 0.2395, 1.0,
  0.3612, 1.0000, 0.2341, 1.0,
  0.3733, 1.0000, 0.2287, 1.0,
  0.3853, 1.0000, 0.2233, 1.0,
  0.3974, 1.0000, 0.2179, 1.0,
  0.4094, 1.0000, 0.2125, 1.0,
  0.4215, 1.0000, 0.2072, 1.0,
  0.4335, 1.0000, 0.2018, 1.0,
  0.4455, 1.0000, 0.1964, 1.0,
  0.4579, 0.9997, 0.1910, 1.0,
  0.4711, 0.9985, 0.1861, 1.0,
  0.4844, 0.9972, 0.1811, 1.0,
  0.4977, 0.9960, 0.1761, 1.0,
  0.5110, 0.9947, 0.1711, 1.0,
  0.5243, 0.9935, 0.1661, 1.0,
  0.5376, 0.9922, 0.1612, 1.0,
  0.5509, 0.9910, 0.1562, 1.0,
  0.5642, 0.9898, 0.1512, 1.0,
  0.5774, 0.9885, 0.1462, 1.0,
  0.5901, 0.9853, 0.1419, 1.0,
  0.6025, 0.9816, 0.1377, 1.0,
  0.6150, 0.9779, 0.1336, 1.0,
  0.6275, 0.9741, 0.1294, 1.0,
  0.6399, 0.9704, 0.1253, 1.0,
  0.6524, 0.9666, 0.1211, 1.0,
  0.6648, 0.9629, 0.1170, 1.0,
  0.6773, 0.9592, 0.1128, 1.0,
  0.6897, 0.9554, 0.1087, 1.0,
  0.7012, 0.9516, 0.1048, 1.0,
  0.7108, 0.9474, 0.1015, 1.0,
  0.7203, 0.9433, 0.0981, 1.0,
  0.7299, 0.9391, 0.0948, 1.0,
  0.7394, 0.9349, 0.0915, 1.0,
  0.7490, 0.9308, 0.0882, 1.0,
  0.7585, 0.9266, 0.0848, 1.0,
  0.7681, 0.9225, 0.0815, 1.0,
  0.7776, 0.9183, 0.0782, 1.0,
  0.7872, 0.9142, 0.0749, 1.0,
  0.7952, 0.9089, 0.0727, 1.0,
  0.8031, 0.9035, 0.0706, 1.0,
  0.8110, 0.8981, 0.0685, 1.0,
  0.8189, 0.8927, 0.0664, 1.0,
  0.8268, 0.8873, 0.0644, 1.0,
  0.8347, 0.8819, 0.0623, 1.0,
  0.8426, 0.8765, 0.0602, 1.0,
  0.8505, 0.8711, 0.0581, 1.0,
  0.8584, 0.8657, 0.0561, 1.0,
  0.8657, 0.8602, 0.0542, 1.0,
  0.8723, 0.8543, 0.0525, 1.0,
  0.8790, 0.8485, 0.0508, 1.0,
  0.8856, 0.8427, 0.0492, 1.0,
  0.8923, 0.8369, 0.0475, 1.0,
  0.8989, 0.8311, 0.0459, 1.0,
  0.9056, 0.8253, 0.0442, 1.0,
  0.9122, 0.8195, 0.0425, 1.0,
  0.9188, 0.8137, 0.0409, 1.0,
  0.9255, 0.8078, 0.0392, 1.0,
  0.9301, 0.7991, 0.0384, 1.0,
  0.9346, 0.7904, 0.0376, 1.0,
  0.9392, 0.7817, 0.0367, 1.0,
  0.9438, 0.7730, 0.0359, 1.0,
  0.9483, 0.7642, 0.0351, 1.0,
  0.9529, 0.7555, 0.0342, 1.0,
  0.9575, 0.7468, 0.0334, 1.0,
  0.9620, 0.7381, 0.0326, 1.0,
  0.9666, 0.7294, 0.0317, 1.0,
  0.9700, 0.7223, 0.0307, 1.0,
  0.9725, 0.7164, 0.0294, 1.0,
  0.9750, 0.7106, 0.0282, 1.0,
  0.9775, 0.7048, 0.0269, 1.0,
  0.9800, 0.6990, 0.0257, 1.0,
  0.9825, 0.6932, 0.0245, 1.0,
  0.9850, 0.6874, 0.0232, 1.0,
  0.9875, 0.6816, 0.0220, 1.0,
  0.9899, 0.6758, 0.0207, 1.0,
  0.9922, 0.6697, 0.0195, 1.0,
  0.9931, 0.6614, 0.0187, 1.0,
  0.9939, 0.6531, 0.0179, 1.0,
  0.9947, 0.6448, 0.0170, 1.0,
  0.9956, 0.6364, 0.0162, 1.0,
  0.9964, 0.6281, 0.0154, 1.0,
  0.9972, 0.6198, 0.0145, 1.0,
  0.9981, 0.6115, 0.0137, 1.0,
  0.9989, 0.6032, 0.0129, 1.0,
  0.9997, 0.5949, 0.0120, 1.0,
  1.0000, 0.5863, 0.0115, 1.0,
  1.0000, 0.5776, 0.0111, 1.0,
  1.0000, 0.5689, 0.0107, 1.0,
  1.0000, 0.5602, 0.0102, 1.0,
  1.0000, 0.5515, 0.0098, 1.0,
  1.0000, 0.5427, 0.0094, 1.0,
  1.0000, 0.5340, 0.0090, 1.0,
  1.0000, 0.5253, 0.0086, 1.0,
  1.0000, 0.5166, 0.0082, 1.0,
  1.0000, 0.5081, 0.0078, 1.0,
  1.0000, 0.5007, 0.0073, 1.0,
  1.0000, 0.4932, 0.0069, 1.0,
  1.0000, 0.4857, 0.0065, 1.0,
  1.0000, 0.4782, 0.0061, 1.0,
  1.0000, 0.4708, 0.0057, 1.0,
  1.0000, 0.4633, 0.0053, 1.0,
  1.0000, 0.4558, 0.0048, 1.0,
  1.0000, 0.4484, 0.0044, 1.0,
  1.0000, 0.4409, 0.0040, 1.0,
  1.0000, 0.4334, 0.0036, 1.0,
  1.0000, 0.4259, 0.0032, 1.0,
  1.0000, 0.4185, 0.0028, 1.0,
  1.0000, 0.4110, 0.0024, 1.0,
  1.0000, 0.4035, 0.0019, 1.0,
  1.0000, 0.3960, 0.0015, 1.0,
  1.0000, 0.3886, 0.0011, 1.0,
  1.0000, 0.3811, 0.0007, 1.0,
  1.0000, 0.3736, 0.0003, 1.0,
  1.0000, 0.3661, 0.0000, 1.0,
  1.0000, 0.3587, 0.0000, 1.0,
  1.0000, 0.3512, 0.0000, 1.0,
  1.0000, 0.3437, 0.0000, 1.0,
  1.0000, 0.3362, 0.0000, 1.0,
  1.0000, 0.3288, 0.0000, 1.0,
  1.0000, 0.3213, 0.0000, 1.0,
  1.0000, 0.3138, 0.0000, 1.0,
  1.0000, 0.3063, 0.0000, 1.0,
  1.0000, 0.2989, 0.0000, 1.0,
  1.0000, 0.2903, 0.0000, 1.0,
  1.0000, 0.2816, 0.0000, 1.0,
  1.0000, 0.2728, 0.0000, 1.0,
  1.0000, 0.2641, 0.0000, 1.0,
  1.0000, 0.2554, 0.0000, 1.0,
  1.0000, 0.2467, 0.0000, 1.0,
  1.0000, 0.2380, 0.0000, 1.0,
  1.0000, 0.2293, 0.0000, 1.0,
  1.0000, 0.2205, 0.0000, 1.0,
  1.0000, 0.2055, 0.0000, 1.0,
  1.0000, 0.1827, 0.0000, 1.0,
  1.0000, 0.1599, 0.0000, 1.0,
  1.0000, 0.1370, 0.0000, 1.0,
  1.0000, 0.1142, 0.0000, 1.0,
  1.0000, 0.0913, 0.0000, 1.0,
  1.0000, 0.0685, 0.0000, 1.0,
  1.0000, 0.0457, 0.0000, 1.0,
  1.0000, 0.0228, 0.0000, 1.0,
  1.0000, 0.0000, 0.0000, 1.0
};

// Create default colormaps.
void SRInterface::generateColormaps()
{
  size_t rainbowArraySize = sizeof(rainbowRaw) / sizeof(*rainbowRaw);

  std::vector<uint8_t> rainbow;
  rainbow.reserve(rainbowArraySize);
  for (int i = 0; i < rainbowArraySize; i++)
  {
    rainbow.push_back(static_cast<uint8_t>(rainbowRaw[i] * 255.0f));
  }

  // Build rainbow texture (eyetracking version -- will need to change).
  GL(glGenTextures(1, &mRainbowCMap));
  GL(glBindTexture(GL_TEXTURE_1D, mRainbowCMap));
  GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
  GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
  GL(glTexImage1D(GL_TEXTURE_1D, 0,
                  GL_RGBA8,
                  static_cast<GLsizei>(rainbow.size() / 4), 0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE, &rainbow[0]));

  // build grayscale texture.
  const int grayscaleSize = 255 * 4;
  std::vector<uint8_t> grayscale;
  grayscale.reserve(grayscaleSize);
  for (int i = 0; i < 255; i++)
  {
    grayscale.push_back(i);
    grayscale.push_back(i);
    grayscale.push_back(i);
    grayscale.push_back(255);
  }

  // Grayscale texture.
  GL(glGenTextures(1, &mGrayscaleCMap));
  GL(glBindTexture(GL_TEXTURE_1D, mGrayscaleCMap));
  GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
  GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
	GL(glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
  GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
  GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
  GL(glTexImage1D(GL_TEXTURE_1D, 0,
                  GL_RGBA8,
                  static_cast<GLsizei>(grayscale.size() / 4), 0,
                  GL_RGBA,
                  GL_UNSIGNED_BYTE, &grayscale[0]));
}


} // namespace Gui
} // namespace SCIRun 

