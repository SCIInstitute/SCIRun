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


// Simple function to handle object transformations so that the GPU does not
// need to do the same calculation for each vertex.
static void lambdaUniformObjTrafs(::spire::ObjectLambdaInterface& iface, 
                                  std::list<::spire::Interface::UnsatisfiedUniform>& unsatisfiedUniforms)
{
  // Cache object to world transform.
  ::spire::M44 objToWorld = iface.getObjectMetadata<::spire::M44>(
      std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()));

  std::string objectTrafoName = std::get<0>(SRCommonUniforms::getObject());
  std::string objectToViewName = std::get<0>(SRCommonUniforms::getObjectToView());
  std::string objectToCamProjName = std::get<0>(SRCommonUniforms::getObjectToCameraToProjection());

  // Loop through the unsatisfied uniforms and see if we can provide any.
  for (auto it = unsatisfiedUniforms.begin(); it != unsatisfiedUniforms.end(); /*nothing*/ )
  {
    if (it->uniformName == objectTrafoName)
    {
      ::spire::LambdaInterface::setUniform<::spire::M44>(it->uniformType, it->uniformName,
                                                     it->shaderLocation, objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToViewName)
    {
      // Grab the inverse view transform.
      ::spire::M44 inverseView = glm::affineInverse(
          iface.getGlobalUniform<::spire::M44>(std::get<0>(SRCommonUniforms::getCameraToWorld())));
      ::spire::LambdaInterface::setUniform<::spire::M44>(it->uniformType, it->uniformName,
                                              it->shaderLocation, inverseView * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToCamProjName)
    {
      ::spire::M44 inverseViewProjection = iface.getGlobalUniform<::spire::M44>(
          std::get<0>(SRCommonUniforms::getToCameraToProjection()));
      ::spire::LambdaInterface::setUniform<::spire::M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, inverseViewProjection * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else
    {
      ++it;
    }
  }
}

//------------------------------------------------------------------------------
SRInterface::SRInterface(std::shared_ptr<spire::Context> context,
                         const std::vector<std::string>& shaderDirs,
                         bool createThread, LogFunction logFP) :
    spire::Interface(context, shaderDirs, createThread, logFP),
    mCamDistance(7.0f),
    mScreenWidth(640),
    mScreenHeight(480),
    mCamAccumPosDown(0.0f, 0.0f, 0.0f),
    mCamAccumPosNow(0.0f, 0.0f, 0.0f),
    mCamera(new SRCamera(*this)),                       // Should come after all vars have been initialized.
    mSciBall(new SciBall(spire::V3(0.0f, 0.0f, 0.0f), 1.0f))   // Should come after all vars have been initialized.
{
  buildAndApplyCameraTransform();
}

//------------------------------------------------------------------------------
SRInterface::~SRInterface()
{
}

//------------------------------------------------------------------------------
void SRInterface::eventResize(size_t width, size_t height)
{
  mScreenWidth = width;
  mScreenHeight = height; 

  // Ensure glViewport is called appropriately.
  spire::Hub::RemoteFunction resizeFun =
      std::bind(spire::InterfaceImplementation::resize, _1, width, height);
  mHub->addFunctionToThreadQueue(resizeFun);
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
  super::makeCurrent();

  std::string objectName = obj->objectName;

  // Check to see if the object already exists in our list. If so, then
  // remove the object. We will re-add it.
  auto foundObject = std::find(mSRObjects.begin(), mSRObjects.end(), objectName);
  if (foundObject != mSRObjects.end())
  {
    // Remove the object from spire.
    super::removeObject(objectName);
    mSRObjects.erase(foundObject);
  }

  // Add the object. Slightly redundant given we might have removed it before,
  // but it keeps a logical ordering of the objects.
  mSRObjects.push_back(objectName);

  // Now we re-add the object to spire.
  addObject(obj->objectName);

  // Add vertex buffer objects.
  for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireVBO& vbo = *it;
    addVBO(vbo.name, vbo.data, vbo.attributeNames);
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
    addIBO(ibo.name, ibo.data, type);
  }

  // Add passes
  for (auto it = obj->mPasses.cbegin(); it != obj->mPasses.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireSubPass& pass = *it;
    addPassToObject(obj->objectName, pass.programName,
                           pass.vboName, pass.iboName, pass.type,
                           pass.passName, SPIRE_DEFAULT_PASS);

    // Add uniforms associated with the pass
    for (auto it = pass.uniforms.begin(); it != pass.uniforms.end(); ++it)
    {
      std::string uniformName = std::get<0>(*it);
      std::shared_ptr<::spire::AbstractUniformStateItem> uniform(std::get<1>(*it));

      // Be sure to always include the pass name as we are updating a
      // subpass of SPIRE_DEFAULT_PASS.
      addObjectPassUniformConcrete(obj->objectName, uniformName, 
                                          uniform, pass.passName);
    }

    // Add gpu state if it has been set.
    if (pass.hasGPUState == true)
      // Be sure to always include the pass name as we are updating a
      // subpass of SPIRE_DEFAULT_PASS.
      addObjectPassGPUState(obj->objectName, pass.gpuState, pass.passName);

    // Add lambda object uniforms to the pass.
    addLambdaObjectUniforms(obj->objectName, lambdaUniformObjTrafs, pass.passName);
  }

  // Add default identity transform to the object globally (instead of
  // per-pass).
  spire::M44 xform;
  xform[3] = ::spire::V4(0.0f, 0.0f, 0.0f, 1.0f);
  addObjectGlobalMetadata(
      obj->objectName, std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()), xform);

  // This must come *after* adding the passes.

  // Now that we have created all of the appropriate passes, get rid of the
  // VBOs and IBOs.
  for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireVBO& vbo = *it;
    removeVBO(vbo.name);
  }

  for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
  {
    const Core::Datatypes::GeometryObject::SpireIBO& ibo = *it;
    removeIBO(ibo.name);
  }

}


//------------------------------------------------------------------------------
void SRInterface::removeAllGeomObjects()
{
  super::makeCurrent();

  for (auto it = mSRObjects.begin(); it != mSRObjects.end(); ++it)
  {
    removeObject(*it);
  }
  mSRObjects.clear();
}



} // namespace Gui
} // namespace SCIRun 

