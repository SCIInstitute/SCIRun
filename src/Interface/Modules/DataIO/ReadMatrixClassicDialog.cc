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


#include <Interface/Modules/DataIO/ReadMatrixClassicDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/DataIO/ReadMatrixClassic.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/ImportExport/GenericIEPlugin.h>
#include <Core/ImportExport/Matrix/MatrixIEPlugin.h>
#include <iostream>
#include <QFileDialog>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;

ReadMatrixClassicDialog::ReadMatrixClassicDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  connect(openFileButton_, SIGNAL(clicked()), this, SLOT(openFile()));
  //addLineEditManager() TODO: investigate these other signals with lineedit.
  connect(fileNameLineEdit_, SIGNAL(editingFinished()), this, SLOT(pushFileNameToState()));
  connect(fileNameLineEdit_, SIGNAL(returnPressed()), this, SLOT(pushFileNameToState()));
  WidgetStyleMixin::setStateVarTooltipWithStyle(fileNameLineEdit_, Variables::Filename.name());
  WidgetStyleMixin::setStateVarTooltipWithStyle(this, Variables::FileTypeName.name());
  WidgetStyleMixin::setStateVarTooltipWithStyle(openFileButton_, Variables::FileTypeName.name());
  WidgetStyleMixin::tabStyle(tabWidget);
  addLineEditManager(scriptInputLineEdit_, Variables::ScriptEnvironmentVariable);
}

void ReadMatrixClassicDialog::pullSpecial()
{
  static MatrixIEPluginManager mgr;
  selectedFilter_ = pullFilename(state_, fileNameLineEdit_, dialogBoxFilterFromFileTypeDescription(mgr));
}

void ReadMatrixClassicDialog::pushFileNameToState()
{
  state_->setValue(Variables::Filename, fileNameLineEdit_->text().trimmed().toStdString());
}

void ReadMatrixClassicDialog::openFile()
{
  auto types = Modules::DataIO::ReadMatrix::fileTypeList();
  QString typesQ(QString::fromStdString(types));
  auto file = QFileDialog::getOpenFileName(this, "Open Matrix File", dialogDirectory(), typesQ, &selectedFilter_);
  if (file.length() > 0)
  {
    auto typeName = fileTypeDescriptionFromDialogBoxFilter(selectedFilter_.toStdString());
    state_->setValue(Variables::FileTypeName, typeName);
    fileNameLineEdit_->setText(file);
    updateRecentFile(file);
    pushFileNameToState();
  }
}
