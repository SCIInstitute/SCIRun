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

#include <Interface/Modules/Visualization/ConvertMatrixToColorMapDialog.h>
#include <Modules/Visualization/ConvertMatrixToColorMap.h>
#include <Core/Algorithms/Base/VariableHelper.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <algorithm>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Modules::Visualization;

namespace
{
  const int ColumnNumber = 0;
  const int Role = 1;
}

ConvertMatrixToColorMapDialog::ConvertMatrixToColorMapDialog(const std::string& name,
  ModuleStateHandle state, QWidget* parent /* = nullptr */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  WidgetStyleMixin::tableHeaderStyle(columnTable_);

  addCheckBoxManager(autoDetectCheckBox_, Parameters::AutoDetectColumns);
  connect(autoDetectCheckBox_, &QAbstractButton::toggled, this,
    &ConvertMatrixToColorMapDialog::autoDetectChanged);
}

QComboBox* ConvertMatrixToColorMapDialog::makeRoleComboBox() const
{
  auto* box = new QComboBox();
  for (const auto& role : columnRoleNames())
    box->addItem(QString::fromStdString(role));
  connect(box, qOverload<int>(&QComboBox::currentIndexChanged), this,
    &ConvertMatrixToColorMapDialog::pushColumnRoles);
  return box;
}

void ConvertMatrixToColorMapDialog::setRowCount(int columns)
{
  if (columnTable_->rowCount() == columns)
    return;

  const auto oldRowCount = columnTable_->rowCount();
  columnTable_->setRowCount(columns);
  for (int i = oldRowCount; i < columns; ++i)
  {
    auto* label = new QTableWidgetItem(QString("Column %1").arg(i + 1));
    label->setFlags(label->flags() & ~Qt::ItemIsEditable);
    columnTable_->setItem(i, ColumnNumber, label);
    columnTable_->setCellWidget(i, Role, makeRoleComboBox());
  }
  columnTable_->resizeColumnsToContents();
}

void ConvertMatrixToColorMapDialog::setRoles(const std::vector<std::string>& roles)
{
  const auto rows = std::min(static_cast<int>(roles.size()), columnTable_->rowCount());
  for (int i = 0; i < rows; ++i)
  {
    auto* box = qobject_cast<QComboBox*>(columnTable_->cellWidget(i, Role));
    const auto index = box->findText(QString::fromStdString(roles[i]));
    if (index >= 0)
      box->setCurrentIndex(index);
  }
}

void ConvertMatrixToColorMapDialog::pushColumnRoles()
{
  if (pulling_)
    return;

  VariableList roles;
  for (int i = 0; i < columnTable_->rowCount(); ++i)
  {
    auto* box = qobject_cast<QComboBox*>(columnTable_->cellWidget(i, Role));
    roles.push_back(makeVariable("", box->currentText().toStdString()));
  }
  state_->setValue(Parameters::ColumnRoles, roles);
}

void ConvertMatrixToColorMapDialog::autoDetectChanged(bool autoDetect)
{
  columnTable_->setEnabled(!autoDetect);
  // Turning auto-detection off should start from whatever layout is on screen,
  // not from an empty mapping.
  if (!autoDetect)
    pushColumnRoles();
}

void ConvertMatrixToColorMapDialog::pullSpecial()
{
  const bool autoDetect = state_->getValue(Parameters::AutoDetectColumns).toBool();
  const auto savedRoles = state_->getValue(Parameters::ColumnRoles).toVector();

  // The column count is only known once a matrix has arrived; until then the
  // saved mapping is all there is to show.
  auto columns = transient_value_cast<int>(
    state_->getTransientValue(Parameters::MatrixColumnCount));
  if (columns < 1)
    columns = static_cast<int>(savedRoles.size());

  setRowCount(columns);
  columnTable_->setEnabled(!autoDetect);

  if (autoDetect)
  {
    setRoles(transient_value_cast<std::vector<std::string>>(
      state_->getTransientValue(Parameters::DetectedColumnRoles)));
  }
  else
  {
    std::vector<std::string> roles;
    std::transform(savedRoles.begin(), savedRoles.end(), std::back_inserter(roles),
      [](const Variable& v) { return v.toString(); });
    setRoles(roles);
  }

  if (columns < 1)
    statusLabel_->setText("Execute to list the input matrix columns.");
  else if (autoDetect)
    statusLabel_->setText(QString("Layout detected from %1 columns.").arg(columns));
  else
    statusLabel_->setText("Assign a role to each matrix column.");
}
