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

#include <Interface/Modules/Render/ViewScene.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Datatypes/Geometry.h>
#include <QtGui>
#include "QtGLContext.h"

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

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
    mSpire = std::weak_ptr<SRInterface>(mGLWidget->getSpire());
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
    std::shared_ptr<SRInterface> spire = mSpire.lock();
    if (spire == nullptr)
      return;

#ifndef SPIRE_USE_STD_THREADS
    // In single threaded environments we always need to ensure our context
    // is current before attempting to make calls into spire. These function
    // invokations may result in calls to OpenGL -- but ONLY in singlethreaded
    // environments where we do not need to queue up our requests.
    mGLWidget->makeCurrent();
#endif

    for (auto it = geomData->begin(); it != geomData->end(); ++it)
    {
      boost::shared_ptr<Core::Datatypes::GeometryObject> obj = *it;
      spire->handleGeomObject(obj);
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

