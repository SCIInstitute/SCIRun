/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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

#include <QtGui>

#include <Interface/Modules/Render/ViewScene.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Geometry.h>
#include <Interface/Modules/Render/QtGLContext.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

// Simple function to handle object transformations so that the GPU does not
// need to do the same calculation for each vertex.
static void lambdaUniformObjTrafs(::spire::ObjectLambdaInterface& iface, 
                                  std::list< ::spire::Interface::UnsatisfiedUniform >& unsatisfiedUniforms)
{
  // Cache object to world transform.
  ::spire::M44 objToWorld = iface.getObjectMetadata< ::spire::M44 >(
      std::get<0>(spire_sr::SRCommonAttributes::getObjectToWorldTrafo()));

  std::string objectTrafoName = std::get<0>(spire_sr::SRCommonUniforms::getObject());
  std::string objectToViewName = std::get<0>(spire_sr::SRCommonUniforms::getObjectToView());
  std::string objectToCamProjName = std::get<0>(spire_sr::SRCommonUniforms::getObjectToCameraToProjection());

  // Loop through the unsatisfied uniforms and see if we can provide any.
  for (auto it = unsatisfiedUniforms.begin(); it != unsatisfiedUniforms.end(); /*nothing*/ )
  {
    if (it->uniformName == objectTrafoName)
    {
      ::spire::LambdaInterface::setUniform< ::spire::M44 >(it->uniformType, it->uniformName,
                                                     it->shaderLocation, objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToViewName)
    {
      // Grab the inverse view transform.
      ::spire::M44 inverseView = glm::affineInverse(
          iface.getGlobalUniform< ::spire::M44 >(std::get<0>(::spire_sr::SRCommonUniforms::getCameraToWorld())));
      ::spire::LambdaInterface::setUniform< ::spire::M44 >(it->uniformType, it->uniformName,
                                              it->shaderLocation, inverseView * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToCamProjName)
    {
      ::spire::M44 inverseViewProjection = iface.getGlobalUniform< ::spire::M44 >(
          std::get<0>(::spire_sr::SRCommonUniforms::getToCameraToProjection()));
      ::spire::LambdaInterface::setUniform< ::spire::M44 >(it->uniformType, it->uniformName,
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
ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));

  addToolBar();
  
  // Setup Qt OpenGL widget.
  QGLFormat fmt;
  fmt.setAlpha(true);
  fmt.setRgba(true);
  mGLWidget = new GLWidget(new QtGLContext(fmt));

  if (mGLWidget->isValid())
  {
    // Hook up the GLWidget
    glLayout->addWidget(mGLWidget);
    glLayout->update();

    // Set spire transient value (should no longer be used).
    mSpire = std::weak_ptr<spire_sr::SRInterface>(mGLWidget->getSpire());
  }
  else
  {
    /// \todo Display dialog.
    delete mGLWidget;
  }
}

//------------------------------------------------------------------------------
ViewSceneDialog::~ViewSceneDialog()
{
}

//------------------------------------------------------------------------------
void ViewSceneDialog::closeEvent(QCloseEvent *evt)
{
  glLayout->removeWidget(mGLWidget);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::moduleExecuted()
{
  // Grab the geomData transient value.
  auto geomDataTransient = state_->getTransientValue("geomData");
  if (geomDataTransient && !geomDataTransient->empty())
  {
    auto geomData = optional_any_cast_or_default<boost::shared_ptr<std::list<boost::shared_ptr<Core::Datatypes::GeometryObject>>>>(geomDataTransient);
    if (!geomData)
      return;
    std::shared_ptr<spire_sr::SRInterface> spire = mSpire.lock();
    if (spire == nullptr)
      return;

#ifndef SPIRE_USE_STD_THREADS
    // In single threaded environments we always need to ensure our context
    // is current before attempting to make calls into spire. These function
    // invokations may result in calls to OpenGL -- but ONLY in singlethreaded
    // environments where we do not need to queue up our requests.
    mGLWidget->makeCurrent();
#endif

    // Remove ALL prior objects.
    spire->removeAllObjects();

    // Set directional light source (in world space).
    spire->addGlobalUniform("uLightDirWorld", ::spire::V3(1.0f, 0.0f, 0.0f));

    for (auto it = geomData->begin(); it != geomData->end(); ++it)
    {
      boost::shared_ptr<Core::Datatypes::GeometryObject> obj = *it;

      // Since we simply remove all objects from the scene everyframe (that
      // needs to change) we don't need to remove the objects one-by-one.
      //// Try/catch is for single-threaded cases. Exceptions are handled on the
      //// spire thread when spire is threaded.
      //try
      //{
      //  // Will remove all traces of old VBO's / IBO's not in use.
      //  // (remember, we remove the VBOs/IBOs we added at the end of this loop,
      //  //  this is to ensure there is only 1 shared_ptr reference to the IBOs
      //  //  and VBOs in Spire).
      //  spire->removeObject(obj->objectName);
      //}
      //catch (std::out_of_range&)
      //{
      //  // Ignore
      //}

      spire->addObject(obj->objectName);

      // Add vertex buffer objects.
      for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
      {
        const GeometryObject::SpireVBO& vbo = *it;
        spire->addVBO(vbo.name, vbo.data, vbo.attributeNames);
      }

      // Add index buffer objects.
      for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
      {
        const GeometryObject::SpireIBO& ibo = *it;
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
        spire->addIBO(ibo.name, ibo.data, type);
      }

      // Add passes
      for (auto it = obj->mPasses.cbegin(); it != obj->mPasses.cend(); ++it)
      {
        const GeometryObject::SpireSubPass& pass = *it;
        spire->addPassToObject(obj->objectName, pass.programName,
                               pass.vboName, pass.iboName, pass.type,
                               pass.passName, SPIRE_DEFAULT_PASS);

        // Add uniforms associated with the pass
        for (auto it = pass.uniforms.begin(); it != pass.uniforms.end(); ++it)
        {
          std::string uniformName = std::get<0>(*it);
          std::shared_ptr< ::spire::AbstractUniformStateItem > uniform(std::get<1>(*it));

          // Be sure to always include the pass name as we are updating a
          // subpass of SPIRE_DEFAULT_PASS.
          spire->addObjectPassUniformConcrete(obj->objectName, uniformName, 
                                                uniform, pass.passName);
        }

        // Add gpu state if it has been set.
        if (pass.hasGPUState == true)
          // Be sure to always include the pass name as we are updating a
          // subpass of SPIRE_DEFAULT_PASS.
          spire->addObjectPassGPUState(obj->objectName, pass.gpuState, pass.passName);

        // Add lambda object uniforms to the pass.
        spire->addLambdaObjectUniforms(obj->objectName, lambdaUniformObjTrafs, pass.passName);
      }

      // Add default identity transform to the object globally (instead of
      // per-pass).
      ::spire::M44 xform;
      xform[3] = ::spire::V4(0.0f, 0.0f, 0.0f, 1.0f);
      spire->addObjectGlobalMetadata(
        obj->objectName, std::get<0>(spire_sr::SRCommonAttributes::getObjectToWorldTrafo()), xform);

      // This must come *after* adding the passes.

      // Now that we have created all of the appropriate passes, get rid of the
      // VBOs and IBOs.
      for (auto it = obj->mVBOs.cbegin(); it != obj->mVBOs.cend(); ++it)
      {
        const GeometryObject::SpireVBO& vbo = *it;
        spire->removeVBO(vbo.name);
      }

      for (auto it = obj->mIBOs.cbegin(); it != obj->mIBOs.cend(); ++it)
      {
        const GeometryObject::SpireIBO& ibo = *it;
        spire->removeIBO(ibo.name);
      }
    }
  }
}

void ViewSceneDialog::addToolBar() 
{
  auto tools = new QToolBar(this);
  auto menu = new QComboBox(this);
  menu->addItem("Legacy Mouse Control");
  menu->addItem("New Mouse Control");
  tools->addWidget(menu);
  //TODO: hook up to slots. for now, disable.
  menu->setEnabled(false);
  glLayout->addWidget(tools);
}
