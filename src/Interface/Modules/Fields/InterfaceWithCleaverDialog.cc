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
  connect(VerboseCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(AbsoluteVolumeScalingRadioButton_, SIGNAL(clicked()), this, SLOT(push()));
  connect(RelativeVolumeScalingRadioButton_, SIGNAL(clicked()), this, SLOT(push()));
  connect(PaddingCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  connect(VolumeScalingSpinBox_X_, SIGNAL(valueChanged(double)), this, SLOT(push()));
  connect(VolumeScalingSpinBox_Y_, SIGNAL(valueChanged(double)), this, SLOT(push()));
  connect(VolumeScalingSpinBox_Z_, SIGNAL(valueChanged(double)), this, SLOT(push()));
}

void InterfaceWithCleaverDialog::push()
{

 if (!pulling_)
  {
   state_->setValue(InterfaceWithCleaverAlgorithm::VerboseCheckBox, VerboseCheckBox_->isChecked());
   //state_->setValue(InterfaceWithCleaverAlgorithm::PaddingCheckBox, PaddingCheckBox_->isChecked());
   state_->setValue(InterfaceWithCleaverAlgorithm::PaddingCheckBox,true);
   state_->setValue(InterfaceWithCleaverAlgorithm::AbsoluteVolumeScalingRadioButton, AbsoluteVolumeScalingRadioButton_->isChecked()); 
   state_->setValue(InterfaceWithCleaverAlgorithm::RelativeVolumeScalingRadioButton, RelativeVolumeScalingRadioButton_->isChecked());  
   state_->setValue(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_X, VolumeScalingSpinBox_X_->value());
   state_->setValue(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Y, VolumeScalingSpinBox_Y_->value());
   state_->setValue(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Z, VolumeScalingSpinBox_Z_->value());   
  }


}

void InterfaceWithCleaverDialog::pull()
{

  Pulling p(this);
    
  double newValue = state_->getValue(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_X).getDouble();
  if (newValue != VolumeScalingSpinBox_X_->value())
    VolumeScalingSpinBox_X_->setValue(newValue); 
  
  newValue = state_->getValue(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Y).getDouble();
  if (newValue != VolumeScalingSpinBox_Y_->value())
    VolumeScalingSpinBox_Y_->setValue(newValue); 
  
  newValue = state_->getValue(InterfaceWithCleaverAlgorithm::VolumeScalingSpinBox_Z).getDouble();
  if (newValue != VolumeScalingSpinBox_Z_->value())
    VolumeScalingSpinBox_Z_->setValue(newValue);    
  
  VerboseCheckBox_->setChecked(state_->getValue(InterfaceWithCleaverAlgorithm::VerboseCheckBox).getBool());  
  //PaddingCheckBox_->setChecked(state_->getValue(InterfaceWithCleaverAlgorithm::PaddingCheckBox).getBool());
  PaddingCheckBox_->setChecked(true);
  AbsoluteVolumeScalingRadioButton_->setChecked(state_->getValue(InterfaceWithCleaverAlgorithm::AbsoluteVolumeScalingRadioButton).getBool());
  RelativeVolumeScalingRadioButton_->setChecked(state_->getValue(InterfaceWithCleaverAlgorithm::RelativeVolumeScalingRadioButton).getBool());
    
}


void InterfaceWithCleaverDialog::pullAndDisplayInfo() 
{
 /* auto info = optional_any_cast_or_default<ReportFieldInfoAlgorithm::Outputs>(state_->getTransientValue("ReportedInfo"));

  std::ostringstream ostr;
  ostr << "Type: " << info.type << std::endl;
  ostr << "Center: " << info.center << std::endl;
  ostr << "Size: " << info.size << std::endl;
  ostr << "Data min,max: " << info.dataMin << " , " << info.dataMax << std::endl;
  ostr << "# nodes: " << info.numnodes_ << std::endl;
  ostr << "# elements: " << info.numelements_ << std::endl;
  ostr << "# data: " << info.numdata_ << std::endl;
  ostr << "Data location: " << info.dataLocation << std::endl;
  ostr << "Dims (x,y,z): " << info.dims << std::endl;
  ostr << "Geometric size: " << info.geometricSize << std::endl;

  fieldInfoTextEdit_->setPlainText(ostr.str().c_str());*/
}
