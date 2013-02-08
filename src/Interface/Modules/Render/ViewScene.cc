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
#include <QFileDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

//------------------------------------------------------------------------------
ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));

  // Setup Qt OpenGL widget.
  QGLFormat fmt;
  fmt.setAlpha(true);
  fmt.setRgba(true);
  mGLWidget = new GLWidget(fmt);

  // Hook up the GLWidget
  glLayout->addWidget(mGLWidget);
  glLayout->update();

  // Grab the context and pass that to the module (via the state).
  // (should no longer be used).
  std::weak_ptr<Spire::Context> ctx = std::weak_ptr<Spire::Context>(
      std::dynamic_pointer_cast<Spire::Context>(mGLWidget->getContext()));
  state->setTransientValue("glContext", ctx);

  // Set spire transient value (should no longer be used).
  mSpire = std::weak_ptr<Spire::SCIRun::SRInterface>(mGLWidget->getSpire());
  state->setTransientValue("spire", mSpire);
}

//------------------------------------------------------------------------------
ViewSceneDialog::~ViewSceneDialog()
{
  delete mGLWidget;
}

//------------------------------------------------------------------------------
void ViewSceneDialog::moduleExecuted()
{
  // Grab the geomData transient value.
  boost::any geomDataTransient = state_->getTransientValue("geomData");
  if (!geomDataTransient.empty())
  {
    boost::shared_ptr<Core::Datatypes::GeometryObject> geomData;
    try
    {
      geomData = boost::any_cast<boost::shared_ptr<Core::Datatypes::GeometryObject>>(geomDataTransient);
    }
    catch (const boost::bad_any_cast&)
    {
      //error("Unable to cast boost::any transient value to spire pointer.");
      return;
    }

    // Send buffers to spire...
    std::shared_ptr<Spire::SCIRun::SRInterface> spire = mSpire.lock();
    
    if (geomData->vboCommon != nullptr)
    {
      spire->renderHACKSetCommonVBO(geomData->vboCommon, geomData->vboCommonSize);

      // We want iboFaces and iboEdges to enter as nullptr's if they are.
      spire->renderHACKSetUCFace(geomData->iboFaces, geomData->iboFacesSize);
      if (geomData->useZTest == true)
        spire->renderHACKSetUCFaceColor(Spire::V4(1.0f, 1.0f, 1.0f, 0.4f));
      else
        spire->renderHACKSetUCFaceColor(Spire::V4(1.0f, 1.0f, 1.0f, 0.02f));

      spire->renderHACKSetUCEdge(geomData->iboEdges, geomData->iboEdgesSize);
      if (geomData->useZTest == true)
        spire->renderHACKSetUCEdgeColor(Spire::V4(0.1f, 0.9f, 0.1f, 0.3f));
      else
        spire->renderHACKSetUCEdgeColor(Spire::V4(0.1f, 0.9f, 0.1f, 0.3f));
    }
  }
}
