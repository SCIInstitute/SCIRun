/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Interface/Modules/Matlab/ImportFieldsFromMatlabDialog.h>
#include <Modules/Legacy/Matlab/DataIO/ImportFieldsFromMatlab.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/Legacy/Matlab/DataIO/ExportFieldsToMatlab.h>
#ifndef Q_MOC_RUN
#include <Core/Utils/StringUtil.h>
#endif

using namespace SCIRun::Gui;
using namespace SCIRun::Core;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Matlab;

ImportFieldsFromMatlabDialog::ImportFieldsFromMatlabDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  //fixSize();

  WidgetStyleMixin::tableHeaderStyle(tableWidget);
  addLineEditManager(fileNameLineEdit_, Variables::Filename);
  connect(openFileButton_, SIGNAL(clicked()), this, SLOT(openFile()));
  connect(fileNameLineEdit_, SIGNAL(editingFinished()), this, SLOT(pushFileNameToState()));
  connect(fileNameLineEdit_, SIGNAL(returnPressed()), this, SLOT(pushFileNameToState()));
}

void ImportFieldsFromMatlabDialog::openFile()
{
  auto file = QFileDialog::getOpenFileName(this, "Open Matlab File", dialogDirectory(), "*.mat");
  if (file.length() > 0)
  {
    fileNameLineEdit_->setText(file);
    updateRecentFile(file);
    pushFileNameToState();
  }
}

void ImportFieldsFromMatlabDialog::pushFileNameToState()
{
  state_->setValue(Variables::Filename, fileNameLineEdit_->text().trimmed().toStdString());
}

void ImportFieldsFromMatlabDialog::pullSpecial()
{
  auto infos = toStringVector(state_->getValue(Parameters::FieldInfoStrings).toVector());
  tableWidget->setRowCount(infos.size());
  int row = 0;
  QStringList portList;
  portList << "None" << "Port 1" << "Port 2"  << "Port 3"  << "Port 4"  << "Port 5"  << "Port 6";
  auto choices = toStringVector(state_->getValue(Parameters::PortChoices).toVector());
  for (const auto& info : infos)
  {
    tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(info)));
    auto portBox = new QComboBox();
    portBox->addItems(portList);
    if (!choices.empty())
      portBox->setCurrentIndex(portBox->findText(QString::fromStdString(choices[tableWidget->rowCount() - 1])));
    connect(portBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushPortChoices()));
    tableWidget->setCellWidget(row, 1, portBox);
    ++row;
  }
  tableWidget->resizeColumnsToContents();
}

void ImportFieldsFromMatlabDialog::pushPortChoices()
{
  auto portChoices = makeHomogeneousVariableList([this](size_t i) { return qobject_cast<QComboBox*>(tableWidget->cellWidget(i, 1))->currentText().toStdString(); }, tableWidget->rowCount());
  state_->setValue(Parameters::PortChoices, portChoices);
}
