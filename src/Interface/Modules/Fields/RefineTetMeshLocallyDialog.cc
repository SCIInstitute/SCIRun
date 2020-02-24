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


#include <Interface/Modules/Fields/RefineTetMeshLocallyDialog.h>
#include <Core/Algorithms/Field/RefineTetMeshLocallyAlgorithm.h>
#include <Dataflow/Network/ModuleStateInterface.h>
//#include <boost/bimap.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;


RefineTetMeshLocallyDialog::RefineTetMeshLocallyDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addDoubleSpinBoxManager(IsoValue_, Parameters::RefineTetMeshLocallyIsoValue);
  addDoubleSpinBoxManager(EdgeLength_, Parameters::RefineTetMeshLocallyEdgeLength);
  addDoubleSpinBoxManager(Volume_, Parameters::RefineTetMeshLocallyVolume);
  addDoubleSpinBoxManager(DihedralAngleSmaller_, Parameters::RefineTetMeshLocallyDihedralAngleSmaller);
  addDoubleSpinBoxManager(DihedralAngleBigger_, Parameters::RefineTetMeshLocallyDihedralAngleBigger);
  addCheckBoxManager(DoNoSplitSurfaceTets_, Parameters::RefineTetMeshLocallyDoNoSplitSurfaceTets);
  addCheckBoxManager(CounterClockWiseTetOrdering_, Parameters::RefineTetMeshLocallyCounterClockWiseOrdering);
  addCheckBoxManager(UseModuleInputField_, Parameters::RefineTetMeshLocallyUseModuleInputField);
  addSpinBoxManager(MaxNumberRefinementIterations_, Parameters::RefineTetMeshLocallyMaxNumberRefinementIterations);
  addRadioButtonGroupManager({ IsoValueRadioButton_, EdgeLengthRadioButton_, VolumeRadioButton_, DihedralAngleRadioButton_}, Parameters::RefineTetMeshLocallyRadioButtons);
/*
  addComboBoxManager(constraintComboBox_, Parameters::AddConstraints, impl_->refineNameLookup_);
  addComboBoxManager(refinementComboBox_, Parameters::RefineMethod);
  addDoubleSpinBoxManager(isoValueSpinBox_, Parameters::IsoValue);

  connect(constraintComboBox_, SIGNAL(activated(int)), this, SLOT(setIsoValueEnabled()));*/
}
