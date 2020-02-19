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


#include <Interface/Modules/Fields/MapFieldDataFromSourceToDestinationDialog.h>
#include <Core/Algorithms/Legacy/Fields/Mapping/MapFieldDataFromSourceToDestination.h>
#include <Dataflow/Network/ModuleStateInterface.h>  ///TODO: extract into intermediate
#include <Core/Logging/Log.h>
#include <Core/Math/MiscMath.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

namespace SCIRun {
  namespace Gui {
    class MapFieldDataFromSourceToDestinationDialogImpl
    {
    public:
      MapFieldDataFromSourceToDestinationDialogImpl()
      {
        mappingNameLookup_.insert(StringPair("Linear (\"weighted\")", "interpolateddata"));
        mappingNameLookup_.insert(StringPair("Constant mapping: each destination gets nearest source value", "closestdata"));
        mappingNameLookup_.insert(StringPair("Constant mapping: each source projects to just one destination", "singledestination"));
      }
      GuiStringTranslationMap mappingNameLookup_;
    };
  }}

MapFieldDataFromSourceToDestinationDialog::MapFieldDataFromSourceToDestinationDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  impl_(new MapFieldDataFromSourceToDestinationDialogImpl)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addComboBoxManager(methodComboBox_, Parameters::MappingMethod, impl_->mappingNameLookup_);
  addDoubleSpinBoxManager(maxDistanceSpinBox_, Parameters::MaxDistance);
  addDoubleSpinBoxManager(defaultValueDoubleSpinBox_, Parameters::DefaultValue);
  connect(noMaxCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(setNoMaximumValue(int)));
  connect(useNanForUnassignedValuesCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(setUseNanForUnassignedValues(int)));
}

void MapFieldDataFromSourceToDestinationDialog::pullSpecial()
{
  if (IsNan(state_->getValue(Parameters::DefaultValue).toDouble()))
  {
    useNanForUnassignedValuesCheckBox_->setChecked(true);
  }
  if (state_->getValue(Parameters::MaxDistance).toDouble() < 0)
  {
    noMaxCheckBox_->setChecked(true);
  }
}

void MapFieldDataFromSourceToDestinationDialog::setNoMaximumValue(int state)
{
  if (!pulling_)
  {
    if (0 != state)
      state_->setValue(Parameters::MaxDistance, -1.0);
  }
}

void MapFieldDataFromSourceToDestinationDialog::setUseNanForUnassignedValues(int state)
{
  if (!pulling_)
  {
    if (0 != state)
      state_->setValue(Parameters::DefaultValue, std::numeric_limits<double>::quiet_NaN());
  }
}
