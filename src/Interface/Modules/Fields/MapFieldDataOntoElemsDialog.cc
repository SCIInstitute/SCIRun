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


#include <Interface/Modules/Fields/MapFieldDataOntoElemsDialog.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataOntoElems.h>
#include <Dataflow/Network/ModuleStateInterface.h>  ///TODO: extract into intermediate
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

MapFieldDataOntoElemsDialog::MapFieldDataOntoElemsDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  addComboBoxManager(quantityComboBox_, Parameters::Quantity);
  addComboBoxManager(interpolationComboBox_, Parameters::InterpolationModel);
  addComboBoxManager(sampleMethodComboBox_, Parameters::SampleMethod);
  addComboBoxManager(samplePointsComboBox_, Parameters::SamplePoints);
  addDoubleSpinBoxManager(outsideValueDoubleSpinBox_, Parameters::OutsideValue);
  addDoubleLineEditManager(maximumDistanceLineEdit_, Parameters::MaxDistance);
  connect(useNanForUnassignedValuesCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(setUseNanForUnassignedValues(int)));
}

void MapFieldDataOntoElemsDialog::pullSpecial()
{
  if (IsNan(state_->getValue(Parameters::OutsideValue).toDouble()))
  {
    useNanForUnassignedValuesCheckBox_->setChecked(true);
  }
}

void MapFieldDataOntoElemsDialog::setUseNanForUnassignedValues(int state)
{
  if (!pulling_)
  {
    if (0 != state)
    state_->setValue(Parameters::OutsideValue, std::numeric_limits<double>::quiet_NaN());
  }
}
