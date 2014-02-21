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

float rainbowRaw[] =
{
  0, 0, 0.0, 0.0,    // First bin is ZERO!  1, 0, 0.761505, 0.0,
  1, 0, 0.718638, 1.0,
  1, 0, 0.675771, 1.0,
  1, 0, 0.632903, 1.0,
  1, 0, 0.590036, 1.0,
  1, 0, 0.547169, 1.0,
  1, 0, 0.504302, 1.0,
  1, 0, 0.461435, 1.0,
  1, 0, 0.418567, 1.0,
  1, 0, 0.3757, 1.0,
  1, 0, 0.332833, 1.0,
  1, 0, 0.289966, 1.0,
  1, 0, 0.247099, 1.0,
  1, 0, 0.204231, 1.0,
  1, 0, 0.161364, 1.0,
  1, 0, 0.118497, 1.0,
  1, 0, 0.07563, 1.0,
  0.999861, 0.008916, 0.074962, 1.0,
  0.999721, 0.017832, 0.074294, 1.0,
  0.999582, 0.026748, 0.073626, 1.0,
  0.999443, 0.035664, 0.072958, 1.0,
  0.999303, 0.044581, 0.07229, 1.0,
  0.999164, 0.053497, 0.071622, 1.0,
  0.999025, 0.062413, 0.070954, 1.0,
  0.998885, 0.071329, 0.070286, 1.0,
  0.998746, 0.080245, 0.069618, 1.0,
  0.998607, 0.089161, 0.06895, 1.0,
  0.998468, 0.098077, 0.068282, 1.0,
  0.998328, 0.106994, 0.067614, 1.0,
  0.998189, 0.11591, 0.066946, 1.0,
  0.99805, 0.124826, 0.066277, 1.0,
  0.99791, 0.133742, 0.065609, 1.0,
  0.997771, 0.142658, 0.064942, 1.0,
  0.997632, 0.151574, 0.064273, 1.0,
  0.997492, 0.16049, 0.063606, 1.0,
  0.997353, 0.169407, 0.062937, 1.0,
  0.997214, 0.178323, 0.06227, 1.0,
  0.997074, 0.187239, 0.061602, 1.0,
  0.996935, 0.196155, 0.060933, 1.0,
  0.996796, 0.205071, 0.060265, 1.0,
  0.996656, 0.213987, 0.059597, 1.0,
  0.996517, 0.222903, 0.058929, 1.0,
  0.996378, 0.23182, 0.058261, 1.0,
  0.996238, 0.240736, 0.057593, 1.0,
  0.996099, 0.249652, 0.056925, 1.0,
  0.99596, 0.258568, 0.056257, 1.0,
  0.995821, 0.267484, 0.055589, 1.0,
  0.995681, 0.2764, 0.054921, 1.0,
  0.995542, 0.285316, 0.054253, 1.0,
  0.995403, 0.294232, 0.053585, 1.0,
  0.995263, 0.303149, 0.052917, 1.0,
  0.995124, 0.312065, 0.052249, 1.0,
  0.994985, 0.320981, 0.051581, 1.0,
  0.994845, 0.329897, 0.050913, 1.0,
  0.994706, 0.338813, 0.050245, 1.0,
  0.994567, 0.347729, 0.049577, 1.0,
  0.994427, 0.356645, 0.048909, 1.0,
  0.994288, 0.365562, 0.048241, 1.0,
  0.994149, 0.374478, 0.047573, 1.0,
  0.994009, 0.383394, 0.046905, 1.0,
  0.99387, 0.39231, 0.046237, 1.0,
  0.993731, 0.401226, 0.045569, 1.0,
  0.993591, 0.410142, 0.044901, 1.0,
  0.993452, 0.419058, 0.044233, 1.0,
  0.993313, 0.427974, 0.043565, 1.0,
  0.993174, 0.436891, 0.042897, 1.0,
  0.993034, 0.445807, 0.042229, 1.0,
  0.992895, 0.454723, 0.041561, 1.0,
  0.992756, 0.463639, 0.040893, 1.0,
  0.992616, 0.472555, 0.040225, 1.0,
  0.992477, 0.481471, 0.039557, 1.0,
  0.992338, 0.490387, 0.038889, 1.0,
  0.992198, 0.499304, 0.038221, 1.0,
  0.992059, 0.50822, 0.037553, 1.0,
  0.99192, 0.517136, 0.036885, 1.0,
  0.99178, 0.526052, 0.036217, 1.0,
  0.991641, 0.534968, 0.035549, 1.0,
  0.991502, 0.543884, 0.034881, 1.0,
  0.991362, 0.5528, 0.034213, 1.0,
  0.991223, 0.561716, 0.033545, 1.0,
  0.991084, 0.570633, 0.032877, 1.0,
  0.990945, 0.579549, 0.032209, 1.0,
  0.990805, 0.588465, 0.031541, 1.0,
  0.990666, 0.597381, 0.030873, 1.0,
  0.990527, 0.606297, 0.030205, 1.0,
  0.990387, 0.615213, 0.029537, 1.0,
  0.990248, 0.624129, 0.028869, 1.0,
  0.990109, 0.633046, 0.028201, 1.0,
  0.989969, 0.641962, 0.027533, 1.0,
  0.98983, 0.650878, 0.026865, 1.0,
  0.989691, 0.659794, 0.026197, 1.0,
  0.989551, 0.66871, 0.025529, 1.0,
  0.989412, 0.677626, 0.024861, 1.0,
  0.989273, 0.686542, 0.024193, 1.0,
  0.989133, 0.695459, 0.023525, 1.0,
  0.988994, 0.704375, 0.022857, 1.0,
  0.988855, 0.713291, 0.022189, 1.0,
  0.988715, 0.722207, 0.021521, 1.0,
  0.988576, 0.731123, 0.020853, 1.0,
  0.988437, 0.740039, 0.020185, 1.0,
  0.988298, 0.748955, 0.019517, 1.0,
  0.988158, 0.757871, 0.018849, 1.0,
  0.988019, 0.766788, 0.018181, 1.0,
  0.98788, 0.775704, 0.017513, 1.0,
  0.98774, 0.78462, 0.016845, 1.0,
  0.987601, 0.793536, 0.016177, 1.0,
  0.987462, 0.802452, 0.015509, 1.0,
  0.987322, 0.811368, 0.014841, 1.0,
  0.987183, 0.820284, 0.014173, 1.0,
  0.987044, 0.829201, 0.013505, 1.0,
  0.986904, 0.838117, 0.012837, 1.0,
  0.986765, 0.847033, 0.012168, 1.0,
  0.986626, 0.855949, 0.011501, 1.0,
  0.986486, 0.864865, 0.010832, 1.0,
  0.986347, 0.873781, 0.010165, 1.0,
  0.986208, 0.882697, 0.009496, 1.0,
  0.986068, 0.891614, 0.008828, 1.0,
  0.985929, 0.90053, 0.00816, 1.0,
  0.98579, 0.909446, 0.007492, 1.0,
  0.985651, 0.918362, 0.006824, 1.0,
  0.985511, 0.927278, 0.006156, 1.0,
  0.985372, 0.936194, 0.005488, 1.0,
  0.985233, 0.94511, 0.00482, 1.0,
  0.985093, 0.954027, 0.004152, 1.0,
  0.984954, 0.962943, 0.003484, 1.0,
  0.984815, 0.971859, 0.002816, 1.0,
  0.984675, 0.980775, 0.002148, 1.0,
  0.984536, 0.989691, 0.00148, 1.0,
  0.969227, 0.989901, 0.00145, 1.0,
  0.953917, 0.990112, 0.00142, 1.0,
  0.938608, 0.990322, 0.00139, 1.0,
  0.923298, 0.990533, 0.001359, 1.0,
  0.907989, 0.990743, 0.001329, 1.0,
  0.89268, 0.990953, 0.001299, 1.0,
  0.87737, 0.991164, 0.001269, 1.0,
  0.862061, 0.991374, 0.001239, 1.0,
  0.846751, 0.991584, 0.001208, 1.0,
  0.831442, 0.991795, 0.001178, 1.0,
  0.816132, 0.992005, 0.001148, 1.0,
  0.800823, 0.992216, 0.001118, 1.0,
  0.785514, 0.992426, 0.001088, 1.0,
  0.770204, 0.992636, 0.001057, 1.0,
  0.754895, 0.992847, 0.001027, 1.0,
  0.739585, 0.993057, 0.000997, 1.0,
  0.724276, 0.993268, 0.000967, 1.0,
  0.708967, 0.993478, 0.000937, 1.0,
  0.693657, 0.993688, 0.000906, 1.0,
  0.678348, 0.993899, 0.000876, 1.0,
  0.663038, 0.994109, 0.000846, 1.0,
  0.647729, 0.99432, 0.000816, 1.0,
  0.63242, 0.99453, 0.000785, 1.0,
  0.61711, 0.99474, 0.000755, 1.0,
  0.601801, 0.994951, 0.000725, 1.0,
  0.586491, 0.995161, 0.000695, 1.0,
  0.571182, 0.995371, 0.000665, 1.0,
  0.555873, 0.995582, 0.000634, 1.0,
  0.540563, 0.995792, 0.000604, 1.0,
  0.525254, 0.996003, 0.000574, 1.0,
  0.509944, 0.996213, 0.000544, 1.0,
  0.494635, 0.996423, 0.000514, 1.0,
  0.479326, 0.996634, 0.000483, 1.0,
  0.464016, 0.996844, 0.000453, 1.0,
  0.448707, 0.997055, 0.000423, 1.0,
  0.433397, 0.997265, 0.000393, 1.0,
  0.418088, 0.997475, 0.000363, 1.0,
  0.402779, 0.997686, 0.000332, 1.0,
  0.387469, 0.997896, 0.000302, 1.0,
  0.37216, 0.998107, 0.000272, 1.0,
  0.35685, 0.998317, 0.000242, 1.0,
  0.341541, 0.998527, 0.000211, 1.0,
  0.326231, 0.998738, 0.000181, 1.0,
  0.310922, 0.998948, 0.000151, 1.0,
  0.295613, 0.999158, 0.000121, 1.0,
  0.280303, 0.999369, 9.1e-05, 1.0,
  0.264994, 0.999579, 6e-05, 1.0,
  0.249684, 0.99979, 3e-05, 1.0,
  0.234375, 1, 0, 1.0,
  0.229492, 0.980642, 0.020833, 1.0,
  0.224609, 0.961284, 0.041667, 1.0,
  0.219727, 0.941926, 0.0625, 1.0,
  0.214844, 0.922568, 0.083333, 1.0,
  0.209961, 0.90321, 0.104167, 1.0,
  0.205078, 0.883852, 0.125, 1.0,
  0.200195, 0.864494, 0.145833, 1.0,
  0.195312, 0.845136, 0.166667, 1.0,
  0.19043, 0.825778, 0.1875, 1.0,
  0.185547, 0.80642, 0.208333, 1.0,
  0.180664, 0.787062, 0.229167, 1.0,
  0.175781, 0.767704, 0.25, 1.0,
  0.170898, 0.748346, 0.270833, 1.0,
  0.166016, 0.728988, 0.291667, 1.0,
  0.161133, 0.70963, 0.3125, 1.0,
  0.15625, 0.690272, 0.333333, 1.0,
  0.151367, 0.670914, 0.354167, 1.0,
  0.146484, 0.651556, 0.375, 1.0,
  0.141602, 0.632198, 0.395833, 1.0,
  0.136719, 0.61284, 0.416667, 1.0,
  0.131836, 0.593482, 0.4375, 1.0,
  0.126953, 0.574124, 0.458333, 1.0,
  0.12207, 0.554767, 0.479167, 1.0,
  0.117188, 0.535408, 0.5, 1.0,
  0.112305, 0.51605, 0.520833, 1.0,
  0.107422, 0.496693, 0.541667, 1.0,
  0.102539, 0.477335, 0.5625, 1.0,
  0.097656, 0.457977, 0.583333, 1.0,
  0.092773, 0.438619, 0.604167, 1.0,
  0.087891, 0.419261, 0.625, 1.0,
  0.083008, 0.399903, 0.645833, 1.0,
  0.078125, 0.380545, 0.666667, 1.0,
  0.073242, 0.361187, 0.6875, 1.0,
  0.068359, 0.341829, 0.708333, 1.0,
  0.063477, 0.322471, 0.729167, 1.0,
  0.058594, 0.303113, 0.75, 1.0,
  0.053711, 0.283755, 0.770833, 1.0,
  0.048828, 0.264397, 0.791667, 1.0,
  0.043945, 0.245039, 0.8125, 1.0,
  0.039062, 0.225681, 0.833333, 1.0,
  0.03418, 0.206323, 0.854167, 1.0,
  0.029297, 0.186965, 0.875, 1.0,
  0.024414, 0.167607, 0.895833, 1.0,
  0.019531, 0.148249, 0.916667, 1.0,
  0.014648, 0.128891, 0.9375, 1.0,
  0.009766, 0.109533, 0.958333, 1.0,
  0.004883, 0.090175, 0.979167, 1.0,
  0, 0.070817, 1, 1.0,
  0.040136, 0.066391, 1, 1.0,
  0.080272, 0.061965, 1, 1.0,
  0.120409, 0.057539, 1, 1.0,
  0.160545, 0.053113, 1, 1.0,
  0.200681, 0.048687, 1, 1.0,
  0.240817, 0.044261, 1, 1.0,
  0.280954, 0.039835, 1, 1.0,
  0.32109, 0.035409, 1, 1.0,
  0.361226, 0.030982, 1, 1.0,
  0.401362, 0.026556, 1, 1.0,
  0.441498, 0.02213, 1, 1.0,
  0.481635, 0.017704, 1, 1.0,
  0.521771, 0.013278, 1, 1.0,
  0.561907, 0.008852, 1, 1.0,
  0.602043, 0.004426, 1, 1.0,
  0.642179, 0, 1, 1.0,
  0.614089, 0, 0.961854, 1.0,
  0.585999, 0, 0.923708, 1.0,
  0.557909, 0, 0.885561, 1.0,
  0.529819, 0, 0.847415, 1.0,
  0.501729, 0, 0.809269, 1.0,
  0.473639, 0, 0.771123, 1.0,
  0.445548, 0, 0.732977, 1.0,
  0.417458, 0, 0.694831, 1.0,
  0.389368, 0, 0.656684, 1.0,
  0.361278, 0, 0.618538, 1.0,
  0.333188, 0, 0.580392, 1.0,
  0.305098, 0, 0.542246, 1.0,
  0.277008, 0, 0.5041, 1.0,
  0.248917, 0, 0.465954, 1.0,
  0.220827, 0, 0.427807, 1.0
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

  // Build rainbow texture (exxon version -- will need to change).
  GL(glGenTextures(1, &mGrayscaleCMap));
  GL(glBindTexture(GL_TEXTURE_1D, mGrayscaleCMap));
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
}


} // namespace Gui
} // namespace SCIRun 

