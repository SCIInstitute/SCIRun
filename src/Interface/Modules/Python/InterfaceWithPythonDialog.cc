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


#include <Interface/Modules/Python/InterfaceWithPythonDialog.h>
#include <Modules/Python/InterfaceWithPython.h>
#include <Modules/Python/PythonObjectForwarder.h>
#include <Interface/Modules/Base/CustomWidgets/CodeEditorWidgets.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Python;

InterfaceWithPythonDialog::InterfaceWithPythonDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  {
    pythonCodePlainTextEdit_ = new CodeEditor(this);
    tabWidget->widget(0)->layout()->addWidget(pythonCodePlainTextEdit_);
    pythonTopLevelCodePlainTextEdit_ = new CodeEditor(this);
    tabWidget->widget(3)->layout()->addWidget(pythonTopLevelCodePlainTextEdit_);
  }

  addPlainTextEditManager(pythonCodePlainTextEdit_, Parameters::PythonCode);
  addPlainTextEditManager(pythonTopLevelCodePlainTextEdit_, Parameters::PythonTopLevelCode);

  addSpinBoxManager(retryAttemptsSpinBox_, Parameters::NumberOfRetries);
  addSpinBoxManager(pollingIntervalSpinBox_, Parameters::PollingIntervalMilliseconds);

  connect(clearObjectPushButton_, SIGNAL(clicked()), this, SLOT(resetObjects()));

  WidgetStyleMixin::tabStyle(tabWidget);
  WidgetStyleMixin::tableHeaderStyle(inputVariableNamesTableWidget_);
  WidgetStyleMixin::tableHeaderStyle(outputVariableNamesTableWidget_);

  setupOutputTableCells();

  connect(pythonDocPushButton_, SIGNAL(clicked()), this, SLOT(loadAPIDocumentation()));
  connect(addMatlabCodeBlockToolButton_, SIGNAL(clicked()), pythonCodePlainTextEdit_, SLOT(insertSpecialCodeBlock()));

  createForceAlwaysExecuteToggleAction();
}

void InterfaceWithPythonDialog::resetObjects()
{
  for (const auto& objName : SCIRun::Modules::Python::InterfaceWithPython::outputNameParameters())
    state_->setTransientValue(state_->getValue(objName).toString(), boost::any());
}

void InterfaceWithPythonDialog::setupOutputTableCells()
{
  auto outputNames = SCIRun::Modules::Python::InterfaceWithPython::outputNameParameters();
  for (int i = 0; i < outputVariableNamesTableWidget_->rowCount(); ++i)
  {
    for (int j = 0; j < outputVariableNamesTableWidget_->columnCount(); ++j)
    {
      if (j == outputVariableNamesTableWidget_->columnCount() - 1)
      {
        auto lineEdit = new QLineEdit;
        addLineEditManager(lineEdit, outputNames[i]);
        outputVariableNamesTableWidget_->setCellWidget(i, outputVariableNamesTableWidget_->columnCount() - 1, lineEdit);
      }
      else
      {
        auto item = outputVariableNamesTableWidget_->item(i, j);
        if (item)
        {
          item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        }
      }
    }
  }
}

void InterfaceWithPythonDialog::updateFromPortChange(int numPorts, const std::string& portId, DynamicPortChange type)
{
  if (type == DynamicPortChange::INITIAL_PORT_CONSTRUCTION)
    return;

  if (type == DynamicPortChange::USER_REMOVED_PORT)
  {
    //TODO: add checkbox for "don't show this again"
    QMessageBox::warning(this, "Warning: possible Python code update required", windowTitle() +
      ": The connection to port " + QString::fromStdString(portId) + " was deleted. The variable name \"" +
      QString::fromStdString(state_->getValue(SCIRun::Core::Algorithms::Name(portId)).toString()) + "\" is no longer valid."
      + " Please update your Python code or input variable table to reflect this.");
  }

  inputVariableNamesTableWidget_->blockSignals(true);

  handleInputTableWidgetRowChange(portId, "Matrix", type);
  handleInputTableWidgetRowChange(portId, "Field", type);
  handleInputTableWidgetRowChange(portId, "String", type);

  inputVariableNamesTableWidget_->resizeColumnsToContents();
  inputVariableNamesTableWidget_->blockSignals(false);
}

void InterfaceWithPythonDialog::handleInputTableWidgetRowChange(const std::string& portId, const std::string& type, DynamicPortChange portChangeType)
{
  const int lineEditColumn = 2;
  syncTableRowsWithDynamicPort(portId, type, inputVariableNamesTableWidget_, lineEditColumn, portChangeType,
    { { 1, [&type](){ return new QTableWidgetItem(QString::fromStdString(type)); } } });
}

void InterfaceWithPythonDialog::loadAPIDocumentation()
{
  openPythonAPIDoc();
}
