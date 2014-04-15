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

#include <Interface/Modules/Fields/JoinFieldsDialog.h>
#include <Modules/Legacy/Fields/JoinFields.h>
#include <Core/Algorithms/Legacy/Fields/MergeFields/JoinFieldsAlgo.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

JoinFieldsDialog::JoinFieldsDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  connect(forcePointCloudCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(mergeDuplicateElementsCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(mergeDuplicateNodesCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(mergeMeshOnlyCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(onlyMergeSameValueCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(nodeToleranceDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(push()));
}

void JoinFieldsDialog::push()
{
  if (!pulling_)
  {
    state_->setValue(JoinFieldsAlgo::MergeNodes, mergeDuplicateNodesCheckBox_->isChecked());
    state_->setValue(JoinFieldsAlgo::MergeElems, mergeDuplicateElementsCheckBox_->isChecked());
    state_->setValue(SCIRun::Modules::Fields::JoinFields::ForcePointCloud, forcePointCloudCheckBox_->isChecked());
    state_->setValue(JoinFieldsAlgo::MakeNoData, mergeMeshOnlyCheckBox_->isChecked());
    state_->setValue(JoinFieldsAlgo::MatchNodeValues, onlyMergeSameValueCheckBox_->isChecked());
    state_->setValue(JoinFieldsAlgo::Tolerance, nodeToleranceDoubleSpinBox_->value());
  }
}

//BIG DAN TODO: extract class for Widget/StateVar interaction. Starting to look like Seg3D code...

void JoinFieldsDialog::pull()
{
  Pulling p(this);
  
  double newValue = state_->getValue(JoinFieldsAlgo::Tolerance).getDouble();
  if (newValue != nodeToleranceDoubleSpinBox_->value())
    nodeToleranceDoubleSpinBox_->setValue(newValue);

  mergeDuplicateNodesCheckBox_->setChecked(state_->getValue(JoinFieldsAlgo::MergeNodes).getBool());
  mergeDuplicateElementsCheckBox_->setChecked(state_->getValue(JoinFieldsAlgo::MergeElems).getBool());
  mergeMeshOnlyCheckBox_->setChecked(state_->getValue(JoinFieldsAlgo::MakeNoData).getBool());
  onlyMergeSameValueCheckBox_->setChecked(state_->getValue(JoinFieldsAlgo::MatchNodeValues).getBool());
  forcePointCloudCheckBox_->setChecked(state_->getValue(SCIRun::Modules::Fields::JoinFields::ForcePointCloud).getBool());
}
