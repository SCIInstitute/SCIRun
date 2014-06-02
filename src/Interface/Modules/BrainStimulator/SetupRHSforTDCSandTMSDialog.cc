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

#include <Interface/Modules/BrainStimulator/SetupRHSforTDCSandTMSDialog.h>
#include <Core/Algorithms/BrainStimulator/SetupRHSforTDCSandTMSAlgorithm.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::BrainStimulator;


SetupRHSforTDCSandTMSDialog::SetupRHSforTDCSandTMSDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
//  QVarient test = tableWidget_5->item(0,1)->text().toDouble(); // I think this is accessing row 0 col 1 == 1

  // connecting all electrodes
//  for (int i=0; i<126; i++)
//  {
      connect(tableWidget_5, SIGNAL(cellChanged(int,int)), this, SLOT(push())); // cellSelected(int, int)
//  }
  
  //  connect(Skull_, SIGNAL(valueChanged(double)), this, SLOT(push()));
  //  connect(CSF_,   SIGNAL(valueChanged(double)), this, SLOT(push()));
  //  connect(GM_,    SIGNAL(valueChanged(double)), this, SLOT(push()));
  //  connect(WM_,    SIGNAL(valueChanged(double)), this, SLOT(push()));
  //  connect(Electrode_, SIGNAL(valueChanged(double)), this, SLOT(push()));
}

void SetupRHSforTDCSandTMSDialog::push()
{
  if (!pulling_)
  {
    
    double test = tableWidget_5->item(0,1)->text().toDouble();
 //   state_->setValue(SetupRHSforTDCSandTMSAlgorithm::Test, test); //tableWidget_5->item(0,1)-
//    state_->setValue(SetupRHSforTDCSandTMSAlgorithm::Test, qvariant_cast<QObject*>(test)); //tableWidget_5->item(0,1)->text().toDouble()
  //    state_->setValue(SetConductivitiesToTetMeshAlgorithm::Skull, Skull_->value());
  //    state_->setValue(SetConductivitiesToTetMeshAlgorithm::CSF,   CSF_->value());
  //    state_->setValue(SetConductivitiesToTetMeshAlgorithm::GM,    GM_->value());
  //    state_->setValue(SetConductivitiesToTetMeshAlgorithm::WM,    WM_->value());
  //    state_->setValue(SetConductivitiesToTetMeshAlgorithm::Electrode, Electrode_->value());
  }
}

void SetupRHSforTDCSandTMSDialog::pull()
{
  Pulling p(this);
}

