/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#include <Interface/Modules/Fields/RefineCleaverMeshDialog.h>
#include <Core/Algorithms/Field/RefineCleaverMeshAlgorithm.h>
#include <Dataflow/Network/ModuleStateInterface.h>
//#include <boost/bimap.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;


RefineCleaverMeshDialog::RefineCleaverMeshDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  
  addDoubleSpinBoxManager(IsoValue_, Parameters::RefineCleaverMeshIsoValue);
  addDoubleSpinBoxManager(EdgeLength_, Parameters::RefineCleaverMeshEdgeLength);
  addDoubleSpinBoxManager(Volume_, Parameters::RefineCleaverMeshVolume);
  addDoubleSpinBoxManager(DihedralAngleSmaller_, Parameters::RefineCleaverMeshDihedralAngleSmaller);
  addDoubleSpinBoxManager(DihedralAngleBigger_, Parameters::RefineCleaverMeshDihedralAngleBigger);
  addCheckBoxManager(DoNoSplitSurfaceTets_, Parameters::RefineCleaverMeshDoNoSplitSurfaceTets);
  
  addRadioButtonGroupManager({ IsoValueRadioButton_, EdgeLengthRadioButton_, VolumeRadioButton_, DihedralAngleRadioButton_}, Parameters::RefineCleaverMeshRadioButtons);
  state_->setValue(Parameters::RefineCleaverMeshRadioButtons, 0);
/*
  addComboBoxManager(constraintComboBox_, Parameters::AddConstraints, impl_->refineNameLookup_);
  addComboBoxManager(refinementComboBox_, Parameters::RefineMethod);
  addDoubleSpinBoxManager(isoValueSpinBox_, Parameters::IsoValue);

  connect(constraintComboBox_, SIGNAL(activated(int)), this, SLOT(setIsoValueEnabled()));*/
}

void RefineCleaverMeshDialog::pullSpecial()
{

}

