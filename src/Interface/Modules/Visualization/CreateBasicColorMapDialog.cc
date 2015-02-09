/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#include <Interface/Modules/Visualization/CreateBasicColorMapDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;

CreateBasicColorMapDialog::CreateBasicColorMapDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  ColorMap cm("Rainbow");
  previewColorMap_->setStyleSheet(buildGradientString(cm));
  addComboBoxManager(colorMapNameComboBox_, Variables::ColorMapName);
  connect(colorMapNameComboBox_,SIGNAL(currentIndexChanged(QString)),this,SLOT(updateColorMapPreview(QString)));
  connect(shiftText_,SIGNAL(textChanged(QString)),this,SLOT(setShiftSlider(QString)));
  connect(resolutionText_,SIGNAL(textChanged(QString)),this,SLOT(setResolutionSlider(QString)));
  connect(shiftSlider_,SIGNAL(valueChanged(int)),this,SLOT(setShiftText(int)));
  connect(resolutionSlider_,SIGNAL(valueChanged(int)),this,SLOT(setResolutionText(int)));
  connect(invertCheck_,SIGNAL(toggled(bool)),this,SLOT(onInvertCheck(bool)));
}

void CreateBasicColorMapDialog::pull()
{
  pull_newVersionToReplaceOld();
}

void CreateBasicColorMapDialog::updateColorMapPreview(QString s) {
    ColorMap cm(s.toStdString(),resolutionSlider_->value(),static_cast<double>(shiftSlider_->value()) / 100.);
    previewColorMap_->setStyleSheet(buildGradientString(cm));
}


const QString CreateBasicColorMapDialog::buildGradientString(ColorMap cm) {
    std::stringstream ss;
    ss << "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,";
    for (double i = 0.001; i < 1.0; i +=0.001) {
        ss << " stop:" << i;
        ss << " rgba(";
        ColorRGB c = cm.getColorMapVal(i);
        ss << int(255.*c.r()) << ", " << int(255.*c.g()) << ", " << int(255.*c.b()) << ", 255),";
    }
    ss << ");";
    std::string str = ss.str();
    return QString::fromStdString(ss.str());
}


void CreateBasicColorMapDialog::setShiftSlider(QString s) {
    bool ok = false;
    double v = s.toDouble(&ok);
    if (ok && v >= -1. && v <= 1.) {
        shiftSlider_->setValue(static_cast<int>(v * 100.));
        updateColorMapPreview(colorMapNameComboBox_->currentText());
    }
}

void CreateBasicColorMapDialog::setResolutionSlider(QString s) {
    bool ok = false;
    int v = s.toInt(&ok,10);
    if (ok && v >= 2 && v <= 256) {
        resolutionSlider_->setValue(v);
        updateColorMapPreview(colorMapNameComboBox_->currentText());
    }
}

void CreateBasicColorMapDialog::setShiftText(int i) {
    std::stringstream ss;
    double v = static_cast<double>(i) / 100.;
    ss << v;
    shiftText_->setText(QString::fromStdString(ss.str()));
    updateColorMapPreview(colorMapNameComboBox_->currentText());
}

void CreateBasicColorMapDialog::setResolutionText(int i) {
    std::stringstream ss;
    ss << i;
    resolutionText_->setText(QString::fromStdString(ss.str()));
    updateColorMapPreview(colorMapNameComboBox_->currentText());
}

void CreateBasicColorMapDialog::onInvertCheck(bool b) {

}





