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

#include <Interface/Modules/Math/BooleanCompareDialog.h>
#include <Core/Algorithms/Math/BooleanCompareAlgo.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <QtGui>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Math;

namespace SCIRun {
    namespace Gui {
        class BooleanCompareDialogImpl
        {
        public:
            BooleanCompareDialogImpl()
            {
                value_1_.insert(StringPair("Elements", "value"));
                value_1_.insert(StringPair("Size", "size"));
                value_1_.insert(StringPair("Norm", "norm"));
                
                value_2_.insert(StringPair("Elements", "value"));
                value_2_.insert(StringPair("Size", "size"));
                value_2_.insert(StringPair("Norm", "norm"));
                
                condition_.insert(StringPair("A is non-zero","boolop"));
                condition_.insert(StringPair("A and B are non-zero (and)","andop"));
                condition_.insert(StringPair("Either A or B is non-zero (or)","orop"));
                condition_.insert(StringPair("A is less than B (<)","lessop"));
                condition_.insert(StringPair("A is less or equal to B (<=)","lesseqop"));
                condition_.insert(StringPair("A is equal to B (==)","eqop"));
                condition_.insert(StringPair("A is greater than B (>)","greatop"));
                condition_.insert(StringPair("A is greater or equal to B (>=)","greateqop"));
                
                then_.insert(StringPair("Return: null","null"));
                then_.insert(StringPair("Return: first input","first"));
                then_.insert(StringPair("Return: second input","second"));
                then_.insert(StringPair("Return: thrid input","third"));
                then_.insert(StringPair("Quit SCIRun","quit"));
                
                else_.insert(StringPair("Return: null","null"));
                else_.insert(StringPair("Return: first input","first"));
                else_.insert(StringPair("Return: second input","second"));
                else_.insert(StringPair("Return: thrid input","third"));
                else_.insert(StringPair("Quit SCIRun","quit"));
            }
            GuiStringTranslationMap value_1_;
            GuiStringTranslationMap value_2_;
            GuiStringTranslationMap condition_;
            GuiStringTranslationMap then_;
            GuiStringTranslationMap else_;
        };
    }}

    
            

BooleanCompareDialog::BooleanCompareDialog(const std::string& name, ModuleStateHandle state,
	QWidget* parent/* = 0*/)
	: ModuleDialogGeneric(state, parent),
    impl_(new BooleanCompareDialogImpl)
{
	setupUi(this);
	setWindowTitle(QString::fromStdString(name));
	fixSize();
    
    
    

    addComboBoxManager(valueBox_1_, Parameters::Value_Option_1,impl_->value_1_);
    addComboBoxManager(valueBox_2_, Parameters::Value_Option_2,impl_->value_2_);
    addComboBoxManager(conditionBox_, Parameters::Comparison_Option,impl_->condition_);
    addComboBoxManager(thenBox_, Parameters::Then_Option,impl_->then_);
    addComboBoxManager(elseBox_, Parameters::Else_Option,impl_->else_);
}
