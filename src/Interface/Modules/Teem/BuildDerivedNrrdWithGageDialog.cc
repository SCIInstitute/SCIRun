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


#include <Interface/Modules/Teem/BuildDerivedNrrdWithGageDialog.h>
#include <Dataflow/Network/ModuleStateInterface.h>  //TODO: extract into intermediate
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/Legacy/Teem/Misc/BuildDerivedNrrdWithGage.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Teem;

BuildDerivedNrrdWithGageDialog::BuildDerivedNrrdWithGageDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  addComboBoxManager(fieldKindComboBox_, Parameters::FieldKind);
  addComboBoxManager(quantityComboBox_, Parameters::Quantity);
  addComboBoxManager(outputTypeComboBox_, Parameters::OType);
  addComboBoxManager(valuesTypeComboBox_, Parameters::ValuesType);
  addComboBoxManager(dTypeComboBox_, Parameters::DType);
  addComboBoxManager(ddTypeComboBox_, Parameters::DDType);
  addLineEditManager(valuesNumParm1LineEdit_, Parameters::ValuesNumParm1);
  addLineEditManager(valuesNumParm2LineEdit_, Parameters::ValuesNumParm2);
  addLineEditManager(valuesNumParm3LineEdit_, Parameters::ValuesNumParm3);
  addLineEditManager(dNumParm1LineEdit_, Parameters::DNumParm1);
  addLineEditManager(dNumParm2LineEdit_, Parameters::DNumParm2);
  addLineEditManager(dNumParm3LineEdit_, Parameters::DNumParm3);
  addLineEditManager(ddNumParm1LineEdit_, Parameters::DDNumParm1);
  addLineEditManager(ddNumParm2LineEdit_, Parameters::DDNumParm2);
  addLineEditManager(ddNumParm3LineEdit_, Parameters::DDNumParm3);
}

void BuildDerivedNrrdWithGageDialog::updateDescriptionLabel(const QString& item)
{
  //TODO
}
