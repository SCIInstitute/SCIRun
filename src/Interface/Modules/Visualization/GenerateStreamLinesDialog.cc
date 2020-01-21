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


#include <Interface/Modules/Visualization/GenerateStreamLinesDialog.h>
#include <Core/Algorithms/Legacy/Fields/StreamLines/GenerateStreamLines.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Fields;

GenerateStreamLinesDialog::GenerateStreamLinesDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  streamlineMethod_.insert(StringPair("Cell Walk", "CellWalk"));
  streamlineMethod_.insert(StringPair("Adams-Bashforth Multi-Step", "AdamsBashforth"));
  streamlineMethod_.insert(StringPair("Heun Method", "Heun"));
  streamlineMethod_.insert(StringPair("Classic 4th Order Runge-Kutta", "RungeKutta"));
  streamlineMethod_.insert(StringPair("Adaptive Runge-Kutta-Fehlberg", "RungeKuttaFehlberg"));

  addSpinBoxManager(maxStepsSpinBox_, Parameters::StreamlineMaxSteps);
  addDoubleSpinBoxManager(toleranceDoubleSpinBox_, Parameters::StreamlineTolerance);
  addDoubleSpinBoxManager(stepSizeDoubleSpinBox_, Parameters::StreamlineStepSize);
  addComboBoxManager(directionComboBox_, Parameters::StreamlineDirection);
  addComboBoxManager(valueComboBox_, Parameters::StreamlineValue);
  addComboBoxManager(methodComboBox_, Parameters::StreamlineMethod, streamlineMethod_);
  addCheckBoxManager(autoParameterCheckBox_, Parameters::AutoParameters);
  addCheckBoxManager(filterColinearCheckBox_, Parameters::RemoveColinearPoints);
  addCheckBoxManager(multithreadedCheckBox_, Parameters::UseMultithreading);
}
