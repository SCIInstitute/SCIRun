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
#include <QPushButton>
#include "QtGLContext.h"

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;

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
  // NOTE: At one point this was required because the renderer was
  // multi-threaded. It is likely we will run into the same issue in the
  // future. Kept for future reference.
  //glLayout->removeWidget(mGLWidget);
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

    for (auto it = geomData->begin(); it != geomData->end(); ++it)
    {
      boost::shared_ptr<Core::Datatypes::GeometryObject> obj = *it;
      spire->handleGeomObject(obj);
    }
  }
}

//------------------------------------------------------------------------------
void ViewSceneDialog::menuMouseControlChanged(int index)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  if (spire == nullptr)
    return;

  if (index == 0)
    spire->setMouseMode(SRInterface::MOUSE_OLDSCIRUN);
  else
    spire->setMouseMode(SRInterface::MOUSE_NEWSCIRUN);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::addToolBar() 
{
  mToolBar = new QToolBar(this);
  auto menu = new QComboBox(this);
  menu->addItem("Legacy Mouse Control");
  menu->addItem("New Mouse Control");

  mToolBar->addWidget(menu);
  mToolBar->addSeparator();

  QPushButton* autoViewBtn = new QPushButton(this);
  autoViewBtn->setText("Auto View");
  autoViewBtn->setAutoDefault(false);
  autoViewBtn->setDefault(false);
  mToolBar->addWidget(autoViewBtn);

  glLayout->addWidget(mToolBar);

  connect(menu, SIGNAL(currentIndexChanged(int)),this, SLOT(menuMouseControlChanged(int)));
}

