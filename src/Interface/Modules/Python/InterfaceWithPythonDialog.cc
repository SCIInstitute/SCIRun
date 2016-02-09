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

#include <Interface/Modules/Python/InterfaceWithPythonDialog.h>
#include <Modules/Python/InterfaceWithPython.h>
#include <Modules/Python/PythonObjectForwarder.h>
#include <Core/Logging/Log.h>

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

  addTextEditManager(pythonCodeTextEdit_, Parameters::PythonCode);

  addSpinBoxManager(retryAttemptsSpinBox_, Parameters::NumberOfRetries);
  addSpinBoxManager(pollingIntervalSpinBox_, Parameters::PollingIntervalMilliseconds);

  connect(clearObjectPushButton_, SIGNAL(clicked()), this, SLOT(resetObjects()));

  WidgetStyleMixin::tabStyle(tabWidget);
  WidgetStyleMixin::tableHeaderStyle(inputVariableNamesTableWidget_);
  WidgetStyleMixin::tableHeaderStyle(outputVariableNamesTableWidget_);

  setOutputTableColumnsReadOnly();

  connect(pythonDocPushButton_, SIGNAL(clicked()), this, SLOT(loadAPIDocumentation()));

  connect(outputVariableNamesTableWidget_, SIGNAL(cellChanged(int, int)), this, SLOT(updateOutputVariable(int, int)));

  auto outputs = SCIRun::Modules::Python::InterfaceWithPython::outputNameParameters();
  for (int i = 0; i < outputVariableNamesTableWidget_->rowCount(); ++i)
  {
    auto lineEdit = new QLineEdit;
    addLineEditManager(lineEdit, outputs[i]);
    outputVariableNamesTableWidget_->setCellWidget(i, outputVariableNamesTableWidget_->columnCount() - 1, lineEdit);
  }
}

void InterfaceWithPythonDialog::resetObjects()
{
  //TODO
  state_->setTransientValue(Parameters::PythonObject, boost::any());
}

void InterfaceWithPythonDialog::setOutputTableColumnsReadOnly()
{
  for (int i = 0; i < outputVariableNamesTableWidget_->rowCount(); ++i)
  {
    for (int j = 0; j < outputVariableNamesTableWidget_->columnCount() - 1; ++j)
    {
      auto item = outputVariableNamesTableWidget_->item(i, j);
      if (item)
      {
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
      }
    }
  }
}

void InterfaceWithPythonDialog::loadAPIDocumentation()
{
  const QString url = "https://github.com/SCIInstitute/SCIRun/wiki/SCIRun-Python-API-0.2";

  if (!QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode)))
    Core::Logging::Log::get() << Core::Logging::ERROR_LOG << "Failed to open SCIRun Python API page.";
}

void InterfaceWithPythonDialog::updateOutputVariable(int row, int col)
{
  qDebug() << "output var changed: " << row << col << outputVariableNamesTableWidget_->item(row, col)->text();
}