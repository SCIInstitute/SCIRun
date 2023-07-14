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


#include <Interface/Modules/Math/AppendMatrixDialog.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Algorithms/Math/AppendMatrix.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

AppendMatrixDialog::AppendMatrixDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  connect(appendRowsButton_, &QPushButton::clicked, this, &AppendMatrixDialog::isRows);
  connect(appendColumnsButton_, &QPushButton::clicked, this, &AppendMatrixDialog::isCols);
}

void AppendMatrixDialog::isRows() const
{
  state_->setValue(Variables::RowsOrColumns, static_cast<int>(AppendMatrixAlgorithm::Option::ROWS));
}

void AppendMatrixDialog::isCols() const
{
  state_->setValue(Variables::RowsOrColumns, static_cast<int>(AppendMatrixAlgorithm::Option::COLUMNS));
}

void AppendMatrixDialog::pullSpecial() //TODO refactor away
{
  //TODO convert to new widget managers
  if (static_cast<int>(AppendMatrixAlgorithm::Option::ROWS) == state_->getValue(Variables::RowsOrColumns).toInt())
    appendRowsButton_->setChecked(true);
  else
    appendColumnsButton_->setChecked(true);
}
