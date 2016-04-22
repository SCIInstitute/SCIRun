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

#include <Interface/Modules/Matlab/ExportFieldsToMatlabDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/Legacy/Matlab/DataIO/ExportFieldsToMatlab.h>
#include <QFileDialog>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
//using namespace SCIRun::Core::Algorithms::Matlab;

ExportFieldsToMatlabDialog::ExportFieldsToMatlabDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
  //,
  //numMatrixPorts_(0), numFieldPorts_(0), numStringPorts_(0)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  WidgetStyleMixin::tableHeaderStyle(tableWidget);
  //addTextEditManager(matlabCodeTextEdit_, Variables::FunctionString);
  //addLineEditManager(matlabPathLineEdit_, Parameters::MatlabPath);
}
//
//namespace TableColumns
//{
//  const int MinColumn = 0;
//
//  const int InputPortID = 0;
//  const int InputName = 1;
//  const int InputDataType = 2;
//  const int InputArrayType = 3;
//
//  const int MaxColumn = InputArrayType + 1;
//}
//
void ExportFieldsToMatlabDialog::updateFromPortChange(int numPorts, const std::string& portName, DynamicPortChange type)
{
  qDebug() << "FOO";
  if (type == DynamicPortChange::INITIAL_PORT_CONSTRUCTION)
    return;

  static const std::string typeName = "Field";
  const int lineEditColumn = 1;
  syncTableRowsWithDynamicPort(portName, typeName, tableWidget, lineEditColumn, type, {}, {});
}


//
//QComboBox* InterfaceWithMatlabDialog::makeInputDataTypeComboBoxItem() const
//{
//  QStringList bcList;
//  bcList << "same as data" << "double" << "etc"; //TODO complete list
//  QComboBox* bcBox = new QComboBox();
//  bcBox->addItems(bcList);
//  bcBox->setCurrentIndex(0);
//  connect(bcBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushMatrixInput()));
//  return bcBox;
//}
//
//
//QComboBox* InterfaceWithMatlabDialog::makeInputArrayTypeComboBoxItem() const
//{
//  QStringList bcList;
//  bcList << "numeric array" << "struct array";
//  QComboBox* bcBox = new QComboBox();
//  bcBox->addItems(bcList);
//  bcBox->setCurrentIndex(0);
//  connect(bcBox, SIGNAL(currentIndexChanged(int)), this, SLOT(pushMatrixInput()));
//  return bcBox;
//}
//
//void InterfaceWithMatlabDialog::pushMatrixInput()
//{
//  qDebug() << "pushMatrixInput";
//}
//
//void InterfaceWithMatlabDialog::pushTableRow(int row)
//{
//  using namespace TableColumns;
//  for (int col = MinColumn; col < MaxColumn; ++col)
//    pushTable(row, col);
//}
//
//void InterfaceWithMatlabDialog::pushTable(int row, int col)
//{
//  qDebug() << "pushTable";
//  using namespace TableColumns;
///*  if (FieldName == col)
//    pushNames();
//  else if (BoundaryCondition == col)
//    pushBoundaryConditions();
//  else if (InsideConductivity == col)
//    pushInsides();
//  else if (OutsideConductivity == col)
//    pushOutsides();
//    */
//}
