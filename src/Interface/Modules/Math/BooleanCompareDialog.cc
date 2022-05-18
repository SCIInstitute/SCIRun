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


#include <Interface/Modules/Math/BooleanCompareDialog.h>
#include <Core/Algorithms/Math/BooleanCompareAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

BooleanCompareDialog::BooleanCompareDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: ModuleDialogGeneric(state, parent)
{
	setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();

  StringPairs value = {{"Elements", "value"},
    {"Size", "size"},
    {"Norm", "norm"}};
  addComboBoxManager(valueBox_1_, Parameters::Value_Option_1, value);
  addComboBoxManager(valueBox_2_, Parameters::Value_Option_2, value);

  addComboBoxManager(conditionBox_, Parameters::Comparison_Option,
    {{"A is non-zero","boolop"},
    {"A and B are non-zero (and)","andop"},
    {"Either A or B is non-zero (or)","orop"},
    {"A is less than B (<)","lessop"},
    {"A is less or equal to B (<=)","lesseqop"},
    {"A is equal to B (==)","eqop"},
    {"A is greater than B (>)","greatop"},
    {"A is greater or equal to B (>=)","greateqop"}});

  StringPairs result = {{"Return: null","null"},
    {"Return: first input","first"},
    {"Return: second input","second"},
    {"Return: third input","third"},
    {"Quit SCIRun","quit"}};
  addComboBoxManager(thenBox_, Parameters::Then_Option, result);
  addComboBoxManager(elseBox_, Parameters::Else_Option, result);
}
