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


#include <Interface/Modules/Math/ReportColumnMatrixMisfitDialog.h>
#include <Core/Algorithms/Math/ColumnMisfitCalculator/ColumnMatrixMisfitCalculator.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Math;

ReportColumnMatrixMisfitDialog::ReportColumnMatrixMisfitDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  methodMap_.insert(StringPair("Correlation Coefficient", "CC"));
  methodMap_.insert(StringPair("Inverse Correlation Coefficient", "CCinv"));
  methodMap_.insert(StringPair("p Norm", "RMS"));
  methodMap_.insert(StringPair("Relative RMS", "RMSrel"));

  addComboBoxManager(methodComboBox_, Parameters::MisfitMethod, methodMap_);
  addDoubleSpinBoxManager(pValueDoubleSpinBox_, Parameters::PValue);
  addDoubleLineEditManager(ccInvLineEdit_, Parameters::ccInv);
  addDoubleLineEditManager(rmsRelLineEdit_, Parameters::rmsRel);
}
