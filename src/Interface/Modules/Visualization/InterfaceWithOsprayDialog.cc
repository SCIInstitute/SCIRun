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

#include <Interface/Modules/Visualization/InterfaceWithOsprayDialog.h>
#include <Modules/Visualization/InterfaceWithOspray.h>
#include <Dataflow/Network/ModuleStateInterface.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Modules::Visualization;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization::Parameters;

InterfaceWithOsprayDialog::InterfaceWithOsprayDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  fixSize();
  addSpinBoxManager(imageHeightSpinBox_, ImageHeight);
  addSpinBoxManager(imageWidthSpinBox_, ImageWidth);
  addSpinBoxManager(framesSpinBox_, FrameCount);

  addDoubleSpinBoxManager(cameraPositionXDoubleSpinBox_, CameraPositionX);
  addDoubleSpinBoxManager(cameraPositionYDoubleSpinBox_, CameraPositionY);
  addDoubleSpinBoxManager(cameraPositionZDoubleSpinBox_, CameraPositionZ);
  addDoubleSpinBoxManager(cameraUpXDoubleSpinBox_, CameraUpX);
  addDoubleSpinBoxManager(cameraUpYDoubleSpinBox_, CameraUpY);
  addDoubleSpinBoxManager(cameraUpZDoubleSpinBox_, CameraUpZ);
  addDoubleSpinBoxManager(cameraViewXDoubleSpinBox_, CameraViewX);
  addDoubleSpinBoxManager(cameraViewYDoubleSpinBox_, CameraViewY);
  addDoubleSpinBoxManager(cameraViewZDoubleSpinBox_, CameraViewZ);
  addDoubleSpinBoxManager(defaultColorRDoubleSpinBox_, DefaultColorR);
  addDoubleSpinBoxManager(defaultColorGDoubleSpinBox_, DefaultColorG);
  addDoubleSpinBoxManager(defaultColorBDoubleSpinBox_, DefaultColorB);
  addDoubleSpinBoxManager(backgroundColorRDoubleSpinBox_, BackgroundColorR);
  addDoubleSpinBoxManager(backgroundColorGDoubleSpinBox_, BackgroundColorG);
  addDoubleSpinBoxManager(backgroundColorBDoubleSpinBox_, BackgroundColorB);
  addCheckBoxManager(showImageCheckBox_, ShowImageInWindow);

  addDoubleSpinBoxManager(lightColorRDoubleSpinBox_, LightColorR);
  addDoubleSpinBoxManager(lightColorGDoubleSpinBox_, LightColorG);
  addDoubleSpinBoxManager(lightColorBDoubleSpinBox_, LightColorB);
  addDoubleSpinBoxManager(lightIntensityDoubleSpinBox_, LightIntensity);
  addCheckBoxManager(lightVisibleCheckBox_, LightVisible);
  addComboBoxManager(lightTypeComboBox_, LightType);

  createExecuteInteractivelyToggleAction();

  state_->connectSpecificStateChanged(Variables::Filename, [this]() { imageFilenameChanged(); });
  connect(this, SIGNAL(imageFilenameChanged()), this, SLOT(showImage()));
  WidgetStyleMixin::tabStyle(tabWidget);
}

void InterfaceWithOsprayDialog::showImage()
{
  if (state_->getValue(ShowImageInWindow).toBool())
  {
    auto filename(QString::fromStdString(transient_value_cast<std::string>(state_->getTransientValue(Variables::Filename))));
    QImage image(filename);
    auto myLabel = new QLabel(parentWidget());
    myLabel->setPixmap(QPixmap::fromImage(image));
    myLabel->resize(myLabel->pixmap()->size());

    auto d = new QDialog(this);
    d->setWindowTitle("Ospray Scene Image");
    auto layout = new QHBoxLayout;
    layout->addWidget(myLabel);
    d->setLayout(layout);
    d->show();
  }
}
