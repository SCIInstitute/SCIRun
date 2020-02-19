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


#include <Interface/Modules/Fields/RefineMeshDialog.h>
#include <Core/Algorithms/Legacy/Fields/RefineMesh/RefineMesh.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <boost/bimap.hpp>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

namespace SCIRun{
  namespace Gui{
    class RefineMeshDialogImpl
    {
    public:
      RefineMeshDialogImpl()
      {
        typedef boost::bimap<std::string, std::string>::value_type strPair;
        refineNameLookup_.insert(strPair("Do not add constraint", "all"));
        refineNameLookup_.insert(strPair("Do not refine nodes/elements with values less than isovalue", "greaterthan"));
        refineNameLookup_.insert(strPair("Do not refine nodes/elements with values unequal to isovalue", "equal"));
        refineNameLookup_.insert(strPair("Do not refine nodes/elements with values greater than isovalue", "lessthan"));
        refineNameLookup_.insert(strPair("Do not refine any elements", "none"));
      }
      boost::bimap<std::string, std::string> refineNameLookup_;
    };
  }
}
RefineMeshDialog::RefineMeshDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  impl_(new RefineMeshDialogImpl)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  addComboBoxManager(constraintComboBox_, Parameters::AddConstraints, impl_->refineNameLookup_);
  addComboBoxManager(refinementComboBox_, Parameters::RefineMethod);
  addDoubleSpinBoxManager(isoValueSpinBox_, Parameters::IsoValue);

  connect(constraintComboBox_, SIGNAL(activated(int)), this, SLOT(setIsoValueEnabled()));
}

void RefineMeshDialog::pullSpecial()
{
  isoValueSpinBox_->setEnabled(constraintComboBox_->currentText().contains("isovalue"));
}

void RefineMeshDialog::setIsoValueEnabled()
{
  if (!pulling_)
  {
    if (constraintComboBox_->currentIndex() != 0)
    {
      isoValueSpinBox_->setEnabled(true);
      state_->getValue(Parameters::IsoValue).toDouble();
    }
    else
      isoValueSpinBox_->setEnabled(false);
  }
}
