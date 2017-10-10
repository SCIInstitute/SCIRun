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
#include <Core/GeometryPrimitives/Point.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
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
  addDoubleSpinBoxManager(defaultColorADoubleSpinBox_, DefaultColorA);
  addDoubleSpinBoxManager(backgroundColorRDoubleSpinBox_, BackgroundColorR);
  addDoubleSpinBoxManager(backgroundColorGDoubleSpinBox_, BackgroundColorG);
  addDoubleSpinBoxManager(backgroundColorBDoubleSpinBox_, BackgroundColorB);
  addDoubleSpinBoxManager(streamlineRadiusDoubleSpinBox_, StreamlineRadius);
  addCheckBoxManager(showImageCheckBox_, ShowImageInWindow);

  addDoubleSpinBoxManager(lightColorRDoubleSpinBox_, LightColorR);
  addDoubleSpinBoxManager(lightColorGDoubleSpinBox_, LightColorG);
  addDoubleSpinBoxManager(lightColorBDoubleSpinBox_, LightColorB);
  addDoubleSpinBoxManager(lightIntensityDoubleSpinBox_, LightIntensity);
  addCheckBoxManager(lightVisibleCheckBox_, LightVisible);
  addComboBoxManager(lightTypeComboBox_, LightType);
  addCheckBoxManager(automaticViewCheckBox_, AutoCameraView);
  connect(automaticViewCheckBox_, SIGNAL(stateChanged(int)), this, SLOT(updateViewWidgets(int)));

  connect(saveFileButton_, SIGNAL(clicked()), this, SLOT(saveFile()));
  connect(fileNameLineEdit_, SIGNAL(editingFinished()), this, SLOT(pushFileNameToState()));
  connect(fileNameLineEdit_, SIGNAL(returnPressed()), this, SLOT(pushFileNameToState()));

  createExecuteInteractivelyToggleAction();

  state_->connectSpecificStateChanged(Variables::Filename, [this]() { imageFilenameChanged(); });
  connect(this, SIGNAL(imageFilenameChanged()), this, SLOT(showImage()));
  WidgetStyleMixin::tabStyle(tabWidget);

  connect(closeImageWindowsPushButton_, SIGNAL(clicked()), this, SLOT(closeImageWindows()));

  connect(zoomHorizontalSlider_, SIGNAL(sliderReleased()), this, SLOT(adjustZoom()));
}

void InterfaceWithOsprayDialog::adjustZoom()
{
  Point camPos(state_->getValue(CameraPositionX).toDouble(), state_->getValue(CameraPositionY).toDouble(), state_->getValue(CameraPositionZ).toDouble());
  Point camView(state_->getValue(CameraViewX).toDouble(), state_->getValue(CameraViewY).toDouble(), state_->getValue(CameraViewZ).toDouble());
  //qDebug() << "camView" << camView.get_string().c_str();
  auto a = camPos - camView;
  //qDebug() << "distance: " << a.length();

  auto zoomFactor = 2 * (zoomHorizontalSlider_->value() + 1.0) / (zoomHorizontalSlider_->maximum() + 1);
  auto newPos = camView + zoomFactor*a;
  state_->setValue(CameraPositionX, newPos.x());
  state_->setValue(CameraPositionY, newPos.y());
  state_->setValue(CameraPositionZ, newPos.z());

  zoomHorizontalSlider_->setValue(5);
  Q_EMIT executeFromStateChangeTriggered();
}

void InterfaceWithOsprayDialog::closeImageWindows()
{
  for (auto& child : children())
  {
    auto dialog = qobject_cast<QDialog*>(child);
    if (dialog)
      dialog->close();
  }
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
    d->setWindowTitle("Ospray Scene Image: " + filename);
    auto layout = new QHBoxLayout;
    layout->addWidget(myLabel);
    d->setLayout(layout);
    d->show();
  }
}

void InterfaceWithOsprayDialog::updateViewWidgets(int state)
{
  manualViewGroupBox_->setEnabled(state == 0);
  manualUpGroupBox_->setEnabled(state == 0);
  autoZoomGroupBox_->setDisabled(state == 0);
}

void InterfaceWithOsprayDialog::pullSpecial()
{
  fileNameLineEdit_->setText(QString::fromStdString(state_->getValue(Variables::Filename).toString()));
}

void InterfaceWithOsprayDialog::pushFileNameToState()
{
  state_->setValue(Variables::Filename, fileNameLineEdit_->text().trimmed().toStdString());
}

void InterfaceWithOsprayDialog::saveFile()
{
  auto dir = QFileDialog::getExistingDirectory(this, "Choose Directory to save images", ".");
  if (dir.length() > 0)
  {
    fileNameLineEdit_->setText(dir);
    pushFileNameToState();
  }
}
