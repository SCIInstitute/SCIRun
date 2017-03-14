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

#include <Interface/Modules/Visualization/ShowStringDialog.h>
#include <Modules/Visualization/ShowString.h>
#include <Core/Datatypes/Color.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Datatypes;

ShowStringDialog::ShowStringDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();

  WidgetStyleMixin::tabStyle(locationTabWidget_);

  connect(colorButton_, SIGNAL(clicked()), this, SLOT(getColor()));

  addSpinBoxManager(fontSizeSpinBox_, Parameters::FontSize);
}

void ShowStringDialog::getColor()
{
  color_ = QColorDialog::getColor(color_, this, "Choose text color");
  setButtonColor();
 
  state_->setValue(Parameters::TextRed, color_.redF());
  state_->setValue(Parameters::TextGreen, color_.greenF());
  state_->setValue(Parameters::TextBlue, color_.blueF());
  //state_->setValue(Parameters::TextAlpha, color_.alphaF());
}

void ShowStringDialog::pullSpecial()
{
  ColorRGB color(state_->getValue(Parameters::TextRed).toDouble(),
    state_->getValue(Parameters::TextGreen).toDouble(),
    state_->getValue(Parameters::TextBlue).toDouble()
    );

  color_ = QColor(
    static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
    static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
    static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));

  setButtonColor();
}

void ShowStringDialog::setButtonColor()
{
  std::stringstream ss;
  ss << "QPushButton { background-color: rgb("
    << color_.red() << ", "
    << color_.green() << ", "
    << color_.blue() <<
    "); color: rgb("
    << 256 - color_.red() << ", "
    << 256 - color_.green() << ", "
    << 256 - color_.blue() << ");}";
  colorButton_->setStyleSheet(QString::fromStdString(ss.str()));
}