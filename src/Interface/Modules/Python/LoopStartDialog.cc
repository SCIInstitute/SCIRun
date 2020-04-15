/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2020 Scientific Computing and Imaging Institute,
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


#include <Interface/Modules/Python/LoopStartDialog.h>
#include <Modules/Python/LoopStart.h>
#include <Interface/Modules/Base/CustomWidgets/CodeEditorWidgets.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Python;

LoopStartDialog::LoopStartDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  {
    auto layout = tabWidget->widget(0)->layout();
    pythonStartCodePlainTextEdit_ = new CodeEditor(this);
    layout->addWidget(new QLabel("Loop initialization code", this));
    layout->addWidget(pythonStartCodePlainTextEdit_);
    pythonIncrementCodePlainTextEdit_ = new CodeEditor(this);
    layout->addWidget(new QLabel("Loop increment code", this));
    layout->addWidget(pythonIncrementCodePlainTextEdit_);
    pythonOutputCodePlainTextEdit_ = new CodeEditor(this);
    layout->addWidget(new QLabel("Loop output code", this));
    layout->addWidget(pythonOutputCodePlainTextEdit_);
  }

  addPlainTextEditManager(pythonStartCodePlainTextEdit_, Parameters::LoopStartCode);
  addPlainTextEditManager(pythonIncrementCodePlainTextEdit_, Parameters::LoopIncrementCode);
  addPlainTextEditManager(pythonOutputCodePlainTextEdit_, Parameters::LoopOutputCode);
  addSpinBoxManager(iterationCountSpinBox_, Parameters::IterationCount);
  connect(resetIterationCountPushButton_, &QPushButton::clicked, [this]() { state_->setValue(Parameters::IterationCount, 0); });

  connect(pythonDocPushButton_, &QPushButton::clicked, []() { openPythonAPIDoc(); });
}
