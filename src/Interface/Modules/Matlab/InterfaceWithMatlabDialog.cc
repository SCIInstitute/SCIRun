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


#include <Interface/Modules/Matlab/InterfaceWithMatlabDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/Legacy/Matlab/Interface/InterfaceWithMatlab.h>
#include <QFileDialog>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Matlab;

InterfaceWithMatlabDialog::InterfaceWithMatlabDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  numMatrixPorts_(0), numFieldPorts_(0), numStringPorts_(0)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  WidgetStyleMixin::tabStyle(matlabEngineTabWidget_);
  WidgetStyleMixin::tabStyle(dataTabWidget_);
  WidgetStyleMixin::tabStyle(matrixTabWidget_);
  WidgetStyleMixin::tableHeaderStyle(matrixInputTableWidget_);
  addTextEditManager(matlabCodeTextEdit_, Variables::FunctionString);
  addLineEditManager(matlabPathLineEdit_, Parameters::MatlabPath);
}

namespace TableColumns
{
  const int MinColumn = 0;

  const int InputPortID = 0;
  const int InputName = 1;
  const int InputDataType = 2;
  const int InputArrayType = 3;

  const int MaxColumn = InputArrayType + 1;
}

void InterfaceWithMatlabDialog::updateFromPortChange(int numPorts, const std::string& portId)
{
  //qDebug() << "ports:" << numPorts << "name: " << QString::fromStdString(portId);
  using namespace TableColumns;

  if (portId.find("Matrix") != std::string::npos)
  {
    //qDebug() << "adjust matrix input table";
    auto oldRowCount = matrixInputTableWidget_->rowCount();
    matrixInputTableWidget_->blockSignals(true);
    if (numPorts > totalInputPorts())
    {
      numMatrixPorts_++;
      auto newRowCount = numMatrixPorts_ - 1;
      if (newRowCount > 0)
      {
        //qDebug() << "oldRowCount" << oldRowCount << "newRowCount" << newRowCount;
        matrixInputTableWidget_->setRowCount(newRowCount);
        int index = newRowCount - 1;
        //note: the incoming portId is the port that was just added, not connected to. we assume the connected port
        // is one index less.
        static boost::regex portIdRegex("InputMatrix\\:(.+)");
        boost::smatch what;
        regex_match(portId, what, portIdRegex);
        const int connectedPortNumber = boost::lexical_cast<int>(what[1]) - 1;
        const std::string connectedPortId = "InputMatrix:" + boost::lexical_cast<std::string>(connectedPortNumber);

        matrixInputTableWidget_->setItem(index, InputPortID, new QTableWidgetItem(QString::fromStdString(connectedPortId)));
        matrixInputTableWidget_->setItem(index, InputName, new QTableWidgetItem("i" + QString::number(connectedPortNumber)));
        matrixInputTableWidget_->setCellWidget(index, InputDataType, makeInputDataTypeComboBoxItem());
        matrixInputTableWidget_->setCellWidget(index, InputArrayType, makeInputArrayTypeComboBoxItem());

        auto port = matrixInputTableWidget_->item(index, InputPortID);
        port->setFlags(port->flags() & ~Qt::ItemIsEditable);
        pushTableRow(index);
        pull();
        matrixInputTableWidget_->resizeColumnsToContents();
      }
    }
    else
    {
      numMatrixPorts_--;
      //qDebug() << "trying to remove row with " << QString::fromStdString(portId);
      auto items = matrixInputTableWidget_->findItems(QString::fromStdString(portId), Qt::MatchFixedString);
      if (!items.empty())
      {
        auto item = items[0];
        int row = matrixInputTableWidget_->row(item);
        matrixInputTableWidget_->removeRow(row);
      }
      else
      {
        qDebug() << "list is empty";
      }
    }
    matrixInputTableWidget_->blockSignals(false);

  }
  else if (portId.find("Field") != std::string::npos)
  {
    //qDebug() << "adjust field input table";
    auto oldRowCount = matrixInputTableWidget_->rowCount();
    if (numPorts > totalInputPorts())
      numFieldPorts_++;
    else
      numFieldPorts_--;
    auto newRowCount = numFieldPorts_ - 1;
    //qDebug() << "oldRowCount" << oldRowCount << "newRowCount" << newRowCount;
  }
  else if (portId.find("String") != std::string::npos)
  {
    //qDebug() << "adjust string input table";
    auto oldRowCount = matrixInputTableWidget_->rowCount();
    if (numPorts > totalInputPorts())
      numStringPorts_++;
    else
      numStringPorts_--;
    auto newRowCount = numStringPorts_ - 1;
    //qDebug() << "oldRowCount" << oldRowCount << "newRowCount" << newRowCount;
  }
  else if (portId.find("Nrrd") != std::string::npos)
  {
    qDebug() << "adjust Nrrd input table";
  }


}

QComboBox* InterfaceWithMatlabDialog::makeInputDataTypeComboBoxItem() const
{
  QStringList bcList;
  bcList << "same as data" << "double" << "etc"; //TODO complete list
  QComboBox* bcBox = new QComboBox();
  bcBox->addItems(bcList);
  bcBox->setCurrentIndex(0);
  connect(bcBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushMatrixInput()));
  return bcBox;
}


QComboBox* InterfaceWithMatlabDialog::makeInputArrayTypeComboBoxItem() const
{
  QStringList bcList;
  bcList << "numeric array" << "struct array";
  QComboBox* bcBox = new QComboBox();
  bcBox->addItems(bcList);
  bcBox->setCurrentIndex(0);
  connect(bcBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushMatrixInput()));
  return bcBox;
}

void InterfaceWithMatlabDialog::pushMatrixInput()
{
  qDebug() << "pushMatrixInput";
}

void InterfaceWithMatlabDialog::pushTableRow(int row)
{
  using namespace TableColumns;
  for (int col = MinColumn; col < MaxColumn; ++col)
    pushTable(row, col);
}

void InterfaceWithMatlabDialog::pushTable(int row, int col)
{
  qDebug() << "pushTable";
  using namespace TableColumns;
/*  if (FieldName == col)
    pushNames();
  else if (BoundaryCondition == col)
    pushBoundaryConditions();
  else if (InsideConductivity == col)
    pushInsides();
  else if (OutsideConductivity == col)
    pushOutsides();
    */
}
#endif
