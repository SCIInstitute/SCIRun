/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Interface/Modules/Fields/ResampleRegularMeshDialog.h>
#include <Modules/Legacy/Fields/ResampleRegularMesh.h>
#include <Core/Algorithms/Legacy/Fields/ResampleMesh/ResampleRegularMesh.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

ResampleRegularMeshDialog::ResampleRegularMeshDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addDoubleSpinBoxManager(gaussianSigmaDoubleSpinBox_, Parameters::ResampleGaussianSigma);
  addDoubleSpinBoxManager(gaussianExtendDoubleSpinBox_, Parameters::ResampleGaussianExtend);
  addDoubleSpinBoxManager(xAxisDoubleSpinBox_, Parameters::ResampleXDim);
  addDoubleSpinBoxManager(yAxisDoubleSpinBox_, Parameters::ResampleYDim);
  addDoubleSpinBoxManager(zAxisDoubleSpinBox_, Parameters::ResampleZDim);
  addTwoChoiceBooleanComboBoxManager(xAxisDimensionComboBox_, Parameters::ResampleXDimUseScalingFactor);
  addTwoChoiceBooleanComboBoxManager(yAxisDimensionComboBox_, Parameters::ResampleYDimUseScalingFactor);
  addTwoChoiceBooleanComboBoxManager(zAxisDimensionComboBox_, Parameters::ResampleZDimUseScalingFactor);
  addComboBoxManager(kernelComboBox_, Parameters::ResampleMethod);

  connect(kernelComboBox_, SIGNAL(activated(const QString&)), this, SLOT(setGuassianWidgetsEnabled(const QString&)));
}

void ResampleRegularMeshDialog::pull()
{
  pull_newVersionToReplaceOld();
}

void ResampleRegularMeshDialog::setGuassianWidgetsEnabled(const QString& label)
{
  bool enable = "Gaussian" == label;
  gaussianSigmaDoubleSpinBox_->setEnabled(enable);
  gaussianExtendDoubleSpinBox_->setEnabled(enable);
  gaussianExtendLabel_->setEnabled(enable);
  gaussianSigmaLabel_->setEnabled(enable);
}