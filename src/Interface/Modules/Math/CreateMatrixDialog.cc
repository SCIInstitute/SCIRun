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


#include <Interface/Modules/Math/CreateMatrixDialog.h>
#include <Modules/Math/CreateMatrix.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Interface/Modules/Base/CustomWidgets/CodeEditorWidgets.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules;

CreateMatrixDialog::CreateMatrixDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), firstPull_(true), matrixTextEdit_(nullptr)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  {
    matrixTextEdit_ = new CodeEditor(this);
    layout()->addWidget(matrixTextEdit_);
    matrixTextEdit_->setEnabled(false);
  }

  connect(editCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(pushMatrixToState(int)));
  connect(matrixTextEdit_, SIGNAL(textChanged()), this, SLOT(editBoxUnsaved()));
  connect(editCheckBox_, SIGNAL(toggled(bool)), matrixTextEdit_, SLOT(setEnabled(bool)));
}

void CreateMatrixDialog::hideEvent(QHideEvent* event)
{
  editCheckBox_->setChecked(false);
}

void CreateMatrixDialog::moduleExecuted()
{
  remindAboutUnsavedMatrix();
}

void CreateMatrixDialog::remindAboutUnsavedMatrix()
{
  if (editCheckBox_->isChecked() && editCheckBox_->text().contains("changed"))
    QMessageBox::information(nullptr, "Matrix input unsaved: " + windowTitle(),
      windowTitle() + "\nMatrix is edited but not saved--did you mean to click the save checkbox?");
}

void CreateMatrixDialog::pushMatrixToState(int state)
{
  if (!pulling_)
  {
    if (0 == state) // matrix is done editing
    {
      state_->setValue(Core::Algorithms::Math::Parameters::TextEntry, matrixTextEdit_->toPlainText().toStdString());
      editBoxSaved();
    }
  }
}

void CreateMatrixDialog::pullSpecial()
{
  Pulling p(this);
  matrixTextEdit_->setPlainText(QString::fromStdString(state_->getValue(Core::Algorithms::Math::Parameters::TextEntry).toString()));
  if (firstPull_)
    editBoxSaved();

  firstPull_ = false;
}

void CreateMatrixDialog::editBoxUnsaved()
{
  editCheckBox_->setText("Matrix changed--click here to save");
}

void CreateMatrixDialog::editBoxSaved()
{
  editCheckBox_->setText("Edit matrix");
}
