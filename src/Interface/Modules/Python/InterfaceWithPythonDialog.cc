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
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

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

  setupOutputTableCells();

  connect(pythonDocPushButton_, SIGNAL(clicked()), this, SLOT(loadAPIDocumentation()));
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

void InterfaceWithPythonDialog::updateFromPortChange(int numPorts, const std::string& portId)
{
  //qDebug() << "updateFromPortChange " << numPorts << QString::fromStdString(portId);
  inputVariableNamesTableWidget_->blockSignals(true);

  handleInputTableWidgetRowChange(numPorts, portId, "Matrix", numMatrixPorts_);
  handleInputTableWidgetRowChange(numPorts, portId, "Field", numFieldPorts_);
  handleInputTableWidgetRowChange(numPorts, portId, "String", numStringPorts_);

  inputVariableNamesTableWidget_->resizeColumnsToContents();
  inputVariableNamesTableWidget_->blockSignals(false);
}

void InterfaceWithPythonDialog::handleInputTableWidgetRowChange(int numPorts, const std::string& portId, const std::string& type, int& portCount)
{
  if (portId.find(type) != std::string::npos)
  {
    //qDebug() << "adjust input table: " << type.c_str();

    inputVariableNamesTableWidget_->blockSignals(true);
    if (numPorts > totalInputPorts())
    {
      //qDebug() << "adding a new table line to " << type.c_str() << portCount;
      portCount++;
      auto newRowCount = portCount - 1;
      if (newRowCount > 0)
      {
        //note: the incoming portId is the port that was just added, not connected to. we assume the connected port
        // is one index less.
        //std::cout << "REGEX: " << "Input" + type + "\\:(.+)" << std::endl;
        boost::regex portIdRegex("Input" + type + "\\:(.+)");
        boost::smatch what;
        //std::cout << "MATCHING WITH: " << portId << std::endl;
        regex_match(portId, what, portIdRegex);
        const int connectedPortNumber = boost::lexical_cast<int>(what[1]) - 1;
        const std::string connectedPortId = "Input" + type + ":" + boost::lexical_cast<std::string>(connectedPortNumber);

        const int rowCount = numPorts - 3;
        inputVariableNamesTableWidget_->setRowCount(rowCount);
        inputVariableNamesTableWidget_->setItem(rowCount - 1, 0, new QTableWidgetItem(QString::fromStdString(connectedPortId)));
        inputVariableNamesTableWidget_->setItem(rowCount - 1, 1, new QTableWidgetItem(QString::fromStdString(type)));

        auto lineEdit = new QLineEdit;
        Core::Algorithms::Name name(connectedPortId);

        if (state_->containsKey(name))
          lineEdit->setText(QString::fromStdString(state_->getValue(name).toString()));
        else
        {
          lineEdit->setText(QString::fromStdString(type).toLower() + "Input" + QString::number(connectedPortNumber + 1));
          state_->setValue(name, lineEdit->text().toStdString());
        }

        addLineEditManager(lineEdit, name);
        inputVariableNamesTableWidget_->setCellWidget(rowCount - 1, 2, lineEdit);
      }
    }
    else
    {
      portCount--;
      //qDebug() << "trying to remove row with " << QString::fromStdString(portId);
      auto items = inputVariableNamesTableWidget_->findItems(QString::fromStdString(portId), Qt::MatchFixedString);
      if (!items.empty())
      {
        auto item = items[0];
        int row = inputVariableNamesTableWidget_->row(item);
        inputVariableNamesTableWidget_->removeRow(row);
        //qDebug() << "row removed" << QString::fromStdString(portId);
        removeManager(Core::Algorithms::Name(portId));
      }
      else
      {
       // qDebug() << "list is empty";
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
