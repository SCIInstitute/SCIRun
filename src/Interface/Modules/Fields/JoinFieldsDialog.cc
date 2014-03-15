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
//#include <Core/Algorithms/Legacy/Fields/DomainFields/GetDomainBoundaryAlgo.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
//using namespace SCIRun::Core::Algorithms::Fields;

JoinFieldsDialog::JoinFieldsDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  //connect(compartmentRadioButton_, SIGNAL(clicked()), this, SLOT(push()));
  //connect(compartmentsRangeRadioButton_, SIGNAL(clicked()), this, SLOT(push()));
  //connect(disconnectBoundariesCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  //connect(excludeInnerBoundaryCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  //connect(includeInnerBoundaryCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
  //connect(maxCompartmentSpinner_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  //connect(minCompartmentSpinner_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  //connect(valueCompartmentSpinner_, SIGNAL(valueChanged(int)), this, SLOT(push()));
  //connect(outerBoundaryCheckBox_, SIGNAL(clicked()), this, SLOT(push()));
}

void JoinFieldsDialog::push()
{
  //if (!pulling_)
  //{
  //  state_->setValue(GetDomainBoundaryAlgo::AddOuterBoundary, outerBoundaryCheckBox_->isChecked());
  //  state_->setValue(GetDomainBoundaryAlgo::InnerBoundaryOnly, includeInnerBoundaryCheckBox_->isChecked());
  //  state_->setValue(GetDomainBoundaryAlgo::DisconnectBoundaries, disconnectBoundariesCheckBox_->isChecked());
  //  state_->setValue(GetDomainBoundaryAlgo::NoInnerBoundary, excludeInnerBoundaryCheckBox_->isChecked());
  //  state_->setValue(GetDomainBoundaryAlgo::UseRange, !compartmentRadioButton_->isChecked());
  //  state_->setValue(GetDomainBoundaryAlgo::UseRange, compartmentsRangeRadioButton_->isChecked());
  //  state_->setValue(GetDomainBoundaryAlgo::MinRange, minCompartmentSpinner_->value());
  //  state_->setValue(GetDomainBoundaryAlgo::MaxRange, maxCompartmentSpinner_->value());
  //  state_->setValue(GetDomainBoundaryAlgo::Domain, valueCompartmentSpinner_->value());
  //}
}

//BIG DAN TODO: extract class for Widget/StateVar interaction. Starting to look like Seg3D code...

void JoinFieldsDialog::pull()
{
  //Pulling p(this);
  //
  //int newValue = state_->getValue(GetDomainBoundaryAlgo::MinRange).getInt();
  //if (newValue != minCompartmentSpinner_->value())
  //  minCompartmentSpinner_->setValue(newValue);

  //newValue = state_->getValue(GetDomainBoundaryAlgo::MaxRange).getInt();
  //if (newValue != maxCompartmentSpinner_->value())
  //  maxCompartmentSpinner_->setValue(newValue);

  //newValue = state_->getValue(GetDomainBoundaryAlgo::Domain).getInt();
  //if (newValue != valueCompartmentSpinner_->value())
  //  valueCompartmentSpinner_->setValue(newValue);
  //
  //outerBoundaryCheckBox_->setChecked(state_->getValue(GetDomainBoundaryAlgo::AddOuterBoundary).getBool());
  //includeInnerBoundaryCheckBox_->setChecked(state_->getValue(GetDomainBoundaryAlgo::InnerBoundaryOnly).getBool());
  //disconnectBoundariesCheckBox_->setChecked(state_->getValue(GetDomainBoundaryAlgo::DisconnectBoundaries).getBool());
  //excludeInnerBoundaryCheckBox_->setChecked(state_->getValue(GetDomainBoundaryAlgo::NoInnerBoundary).getBool());
  //compartmentRadioButton_->setChecked(!state_->getValue(GetDomainBoundaryAlgo::UseRange).getBool());
  //compartmentsRangeRadioButton_->setChecked(state_->getValue(GetDomainBoundaryAlgo::UseRange).getBool());
}
