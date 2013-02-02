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
#include <QFileDialog>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

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
  std::weak_ptr<Spire::Context> ctx = std::weak_ptr<Spire::Context>(
      std::dynamic_pointer_cast<Spire::Context>(mGLWidget->getContext()));
  state->setTransientValue("glContext", ctx);

  // Now use the context to create an instance of Spire and set the Spire
  // transient value.
  std::vector<std::string> shaderDirs;
  mSpire = std::shared_ptr<Spire::Interface>(
      new Spire::Interface(ctx.lock(), shaderDirs, true));
  state->setTransientValue("spire", std::weak_ptr<Spire::Interface>(mSpire));

  // Spire has two separate incoming communication queues. 
  // Spire's behavior upon reading the queues are identical, but there exists
  // two queues so that commands can be sent from two separate threads.

  // There is only one communication queue back from spire primarily because
  // there should only be one thread that is capable of modifying data inside
  // of sci-run.
}

ViewSceneDialog::~ViewSceneDialog()
{
  // Terminate spire and join it's thread.
  mSpire->terminate();
  delete mGLWidget;
}

