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


#include <Interface/Modules/Teem/TendFiberDialog.h>
#include <Modules/Legacy/Teem/Tend/TendFiber.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Teem;

TendFiberDialog::TendFiberDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addDoubleSpinBoxManager(punctureDoubleSpinBox_, Parameters::Puncture);
  addDoubleSpinBoxManager(neighborhoodDoubleSpinBox_, Parameters::Neighborhood);
  addDoubleSpinBoxManager(stepSizeDoubleSpinBox_, Parameters::StepSize);
  addDoubleSpinBoxManager(fiberLengthDoubleSpinBox_, Parameters::FiberLength);
  addDoubleSpinBoxManager(confidenceThresholdDoubleSpinBox_, Parameters::ConfThreshold);
  addDoubleSpinBoxManager(anisotropyThresholdDoubleSpinBox_, Parameters::AnisoThreshold);
  addSpinBoxManager(numStepsSpinBox_, Parameters::Steps);
  addCheckBoxManager(fiberLengthCheckBox_, Parameters::UseLength);
  addCheckBoxManager(numStepsCheckBox_, Parameters::UseSteps);
  addCheckBoxManager(confidenceThresholdCheckBox_, Parameters::UseConf);
  addCheckBoxManager(anisotropyThresholdCheckBox_, Parameters::UseAniso);
  addComboBoxManager(integrationComboBox_, Parameters::Integration);
  addComboBoxManager(fiberAlgoComboBox_, Parameters::FiberType,
    {{"Tensorline (TL)", "tensorline"},
    {"Major eigenvector", "evec1"},
    {"Oriented tensors (OT)", "zhukov"}});
  addComboBoxManager(kernelComboBox_, Parameters::Kernel,
    {{"Tensorline (TL)", "box"},
    {"Major eigenvector", "tent"},
    {"Oriented tensors (OT)", "gaussian"}});
  addComboBoxManager(anisotropyThresholdComboBox_, Parameters::AnisoMetric,
    {{"Tensorline (TL)", "tensorline"},
    {"Major eigenvector", "evec1"},
    {"Oriented tensors (OT)", "zhukov"}});
}
