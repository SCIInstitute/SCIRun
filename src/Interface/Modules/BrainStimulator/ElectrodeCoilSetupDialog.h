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


#ifndef INTERFACE_MODULES_ELECTRODECOILSETUPDIALOG_H
#define INTERFACE_MODULES_ELECTRODECOILSETUPDIALOG_H

#include "Interface/Modules/BrainStimulator/ui_ElectrodeCoilSetupDialog.h"
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/BrainStimulator/share.h>

namespace SCIRun {
namespace Gui {

class SCISHARE ElectrodeCoilSetupDialog : public ModuleDialogGeneric,
  public Ui::ElectrodeCoilSetupDialog
{
	Q_OBJECT

public:
  ElectrodeCoilSetupDialog(const std::string& name,
    SCIRun::Dataflow::Networks::ModuleStateHandle state,
    QWidget* parent = 0);

protected:
  virtual void pullSpecial() override;
private Q_SLOTS:
  void push();
  void validateCell(int row, int col);
  void pushComboBoxChange(int index);

  void pushTable();
  void updateStimTypeColumn();
  void updateInvertNormals();
  void updatePrototypeColumnValues(int index);
  void togglePrototypeColumnReadOnly(int state);
  void toggleThicknessColumnReadOnly(int state);
  void updateThicknessColumnValues(double value);

private:
  bool comboBoxesSetup_;
  bool pushTableFlag_;
  std::vector<QWidget*> inputPortsVector_;
  std::vector<QWidget*> stimTypeVector_;
  void initialize_comboboxes(int i, std::vector<Core::Algorithms::AlgorithmParameter>&  row);
  std::vector<Core::Algorithms::Variable> validate_numerical_input(int i);
  std::vector<Core::Algorithms::Variable> saved_all_elc_values;
};

}
}

#endif
