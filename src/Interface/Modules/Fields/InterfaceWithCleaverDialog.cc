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


#include <Interface/Modules/Fields/InterfaceWithCleaverDialog.h>
#include <Core/Algorithms/Field/InterfaceWithCleaverAlgorithm.h>
#include <Dataflow/Network/ModuleStateInterface.h>  ///TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;


InterfaceWithCleaverDialog::InterfaceWithCleaverDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addDoubleSpinBoxManager(VolumeScalingSpinBox_X_, InterfaceWithCleaverAlgorithm::VolumeScalingX);
  addDoubleSpinBoxManager(VolumeScalingSpinBox_Y_, InterfaceWithCleaverAlgorithm::VolumeScalingY);
  addDoubleSpinBoxManager(VolumeScalingSpinBox_Z_, InterfaceWithCleaverAlgorithm::VolumeScalingZ);
  addCheckBoxManager(VerboseCheckBox_, InterfaceWithCleaverAlgorithm::Verbose);
  addCheckBoxManager(PaddingCheckBox_, InterfaceWithCleaverAlgorithm::Padding);

  addComboBoxManager(scalingComboBox_, InterfaceWithCleaverAlgorithm::VolumeScalingOption);
}
