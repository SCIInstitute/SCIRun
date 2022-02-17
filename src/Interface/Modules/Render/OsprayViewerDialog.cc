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


#include <Interface/Modules/Render/OsprayViewerDialog.h>
#include <Interface/Modules/Render/ES/RendererInterfaceCollaborators.h>
#include <boost/algorithm/string/predicate.hpp>

#ifdef WITH_OSPRAY
#include <ospray/ospray.h>

#include <Modules/Render/ViewScene.h>
#include "Modules/Render/OsprayViewer.h"
#include "Interface/Modules/Render/Ospray/QOSPRayWidget.h"
#include "Interface/Modules/Render/Ospray/OSPRayRenderer.h"
#include "Interface/Modules/Render/ViewOspraySceneConfig.h"

#include <Core/Datatypes/Feedback.h>
#include "Core/Datatypes/Color.h"
#include "Core/Logging/Log.h"
#endif

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
  #ifdef WITH_OSPRAY
using namespace SCIRun::Core::Algorithms::Render;
#endif
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Render;

#pragma GCC diagnostic ignored "-Wunused-parameter"

OsprayViewerDialog::OsprayViewerDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent)
  : ModuleDialogGeneric(state, parent)
{
  #ifdef WITH_OSPRAY
  statusBar_ = new QStatusBar(this);

  renderer_ = new OSPRayRenderer();
  viewer_ = new QOSPRayWidget(parent, renderer_);

  state->connectSpecificStateChanged(Parameters::GeomData, [this]() { Q_EMIT newGeometryValueForwarder(); });
  connect(this, SIGNAL(newGeometryValueForwarder()), this, SLOT(newGeometryValue()));

  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  addConfigurationDialog();
  addToolBar();
  setMinimumSize(200, 200);

  statusBar_->setMaximumHeight(20);
  osprayLayout->addWidget(viewer_);

  addCheckBoxManager(configDialog_->showPlaneCheckBox_, Parameters::ShowPlane);
  addCheckBoxManager(configDialog_->shadowsCheckBox_, Parameters::ShowShadows);
  addCheckBoxManager(configDialog_->renderAnnotationsCheckBox_, Parameters::ShowRenderAnnotations);
  addCheckBoxManager(configDialog_->subsampleCheckBox_, Parameters::SubsampleDuringInteraction);
  addCheckBoxManager(configDialog_->showFrameRateCheckBox_, Parameters::ShowFrameRate);
  addCheckBoxManager(configDialog_->separateModelPerObjectCheckBox_, Parameters::SeparateModelPerObject);
  addCheckBoxManager(configDialog_->ambientVisibleCheckBox_, Parameters::ShowAmbientLight);
  addCheckBoxManager(configDialog_->directionalVisibleCheckBox_, Parameters::ShowDirectionalLight);
  addDoubleSpinBoxManager(configDialog_->autoRotationRateDoubleSpinBox_, Parameters::AutoRotationRate);
  addDoubleSpinBoxManager(configDialog_->cameraViewAtXDoubleSpinBox_, Parameters::CameraViewAtX);
  addDoubleSpinBoxManager(configDialog_->cameraViewAtYDoubleSpinBox_, Parameters::CameraViewAtY);
  addDoubleSpinBoxManager(configDialog_->cameraViewAtZDoubleSpinBox_, Parameters::CameraViewAtZ);
  addDoubleSpinBoxManager(configDialog_->cameraViewFromXDoubleSpinBox_, Parameters::CameraViewFromX);
  addDoubleSpinBoxManager(configDialog_->cameraViewFromYDoubleSpinBox_, Parameters::CameraViewFromY);
  addDoubleSpinBoxManager(configDialog_->cameraViewFromZDoubleSpinBox_, Parameters::CameraViewFromZ);
  addDoubleSpinBoxManager(configDialog_->cameraViewUpXDoubleSpinBox_, Parameters::CameraViewUpX);
  addDoubleSpinBoxManager(configDialog_->cameraViewUpYDoubleSpinBox_, Parameters::CameraViewUpY);
  addDoubleSpinBoxManager(configDialog_->cameraViewUpZDoubleSpinBox_, Parameters::CameraViewUpZ);
  addDoubleSpinBoxManager(configDialog_->directionalLightIntensityDoubleSpinBox_, Parameters::DirectionalLightIntensity);
  addDoubleSpinBoxManager(configDialog_->ambientLightIntensityDoubleSpinBox_, Parameters::AmbientLightIntensity);

  addSpinBoxManager(configDialog_->samplesPerPixelSpinBox_, Parameters::SamplesPerPixel);
  addSpinBoxManager(configDialog_->viewerHeightSpinBox_, Parameters::ViewerHeight);
  addSpinBoxManager(configDialog_->viewerWidthSpinBox_, Parameters::ViewerWidth);

  connect(configDialog_->viewerHeightSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(setHeight(int)));
  connect(configDialog_->viewerWidthSpinBox_, SIGNAL(valueChanged(int)), this, SLOT(setWidth(igeomDataTransientnt)));

  connect(configDialog_->cameraViewAtXDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewAtYDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewAtZDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewFromXDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewFromYDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewFromZDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewUpXDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewUpYDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));
  connect(configDialog_->cameraViewUpZDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setViewportCamera()));

  connect(configDialog_->ambientLightColorRDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setLightColor()));
  connect(configDialog_->ambientLightColorGDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setLightColor()));
  connect(configDialog_->ambientLightColorBDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setLightColor()));
  connect(configDialog_->directionalLightColorRDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setLightColor()));
  connect(configDialog_->directionalLightColorGDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setLightColor()));
  connect(configDialog_->directionalLightColorBDoubleSpinBox_, SIGNAL(valueChanged(double)), this, SLOT(setLightColor()));

  //float tvp[] = {-1.0f,-1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
  //float tvc[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  //uint32_t ind[3] = { 0, 1, 2};

  #endif
}

OsprayViewerDialog::~OsprayViewerDialog()
{
#ifdef WITH_OSPRAY
  delete viewer_;
  delete renderer_;
#endif
}

void OsprayViewerDialog::newGeometryValue()
{
#ifdef WITH_OSPRAY

  auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
  if (!geomDataTransient || geomDataTransient->empty()) return;

  auto geom = transient_value_cast<OsprayGeometryObjectHandle>(geomDataTransient);
  if (!geom) return;

  auto compGeom = std::dynamic_pointer_cast<CompositeOsprayGeometryObject>(geom);

  //TODO pass geometry to the renderer_ in a renderer_ agnostic fashion
  renderer_->updateGeometries(compGeom.get()->objects());
#endif
}

void OsprayViewerDialog::setHeight(int h)
{
  parentWidget()->resize(width(), h);
}

void OsprayViewerDialog::setWidth(int w)
{
  parentWidget()->resize(w, height());
}

void OsprayViewerDialog::addToolBar()
{
  toolBar_ = new QToolBar(this);
  WidgetStyleMixin::toolbarStyle(toolBar_);

  addConfigurationButton();
  addAutoViewButton();
  addAutoRotateButton();
  addTimestepButtons();
  addScreenshotButton();

  osprayLayout->addWidget(toolBar_);

  addControlLockButton();
}

void OsprayViewerDialog::adjustToolbar()
{
  if (toolBar_)
    adjustToolbarForHighResolution(toolBar_);
}

void OsprayViewerDialog::addConfigurationButton()
{
  auto configurationButton = new QPushButton();
  configurationButton->setToolTip("Open/Close Configuration Menu");
  configurationButton->setIcon(QPixmap(":/general/Resources/ViewScene/configure.png"));
  configurationButton->setShortcut(Qt::Key_F5);
  connect(configurationButton, SIGNAL(clicked()), this, SLOT(configButtonClicked()));
  addToolbarButton(configurationButton);
}

void OsprayViewerDialog::configButtonClicked()
{
#ifdef WITH_OSPRAY
  configDialog_->setVisible(!configDialog_->isVisible());
#endif
}

void OsprayViewerDialog::addConfigurationDialog()
{
  #ifdef WITH_OSPRAY
  auto name = windowTitle() + " Configuration";
  configDialog_ = new ViewOspraySceneConfigDialog(name, this);
#endif
}

void OsprayViewerDialog::addToolbarButton(QPushButton* button)
{
  button->setFixedSize(35,35);
  button->setIconSize(QSize(25,25));
  toolBar_->addWidget(button);
}

void OsprayViewerDialog::addAutoViewButton()
{
  autoViewButton_ = new QPushButton(this);

  autoViewButton_->setToolTip("Auto View");
  autoViewButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  autoViewButton_->setShortcut(Qt::Key_0);
  connect(autoViewButton_, SIGNAL(clicked()), this, SLOT(autoViewClicked()));
  addToolbarButton(autoViewButton_);
}

void OsprayViewerDialog::addAutoRotateButton()
{
  autoRotateButton_ = new QPushButton(this);
  autoRotateButton_->setToolTip("Auto Rotate");
  autoRotateButton_->setCheckable(true);
  autoRotateButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autorotate.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(autoRotateButton_, SIGNAL(clicked()), this, SLOT(autoRotateClicked()));
  addToolbarButton(autoRotateButton_);
}

void OsprayViewerDialog::addTimestepButtons()
{
  auto nextTimestep = new QPushButton(this);
  nextTimestep->setText("Next");
  nextTimestep->setToolTip("Next timestep");
  //autoRotateButton->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(nextTimestep, SIGNAL(clicked()), this, SLOT(nextTimestepClicked()));
  addToolbarButton(nextTimestep);

  playTimestepsButton_ = new QPushButton(this);
  playTimestepsButton_->setText("Play");
  playTimestepsButton_->setToolTip("Play timesteps");
  playTimestepsButton_->setCheckable(true);
  //autoRotateButton->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(playTimestepsButton_, SIGNAL(clicked()), this, SLOT(playTimestepsClicked()));
  addToolbarButton(playTimestepsButton_);
}

void OsprayViewerDialog::addScreenshotButton()
{
  auto screenshotButton = new QPushButton(this);
  screenshotButton->setToolTip("Take screenshot");
  screenshotButton->setIcon(QPixmap(":/general/Resources/ViewScene/screenshot.png"));
  screenshotButton->setShortcut(Qt::Key_F12);
  connect(screenshotButton, SIGNAL(clicked()), this, SLOT(screenshotClicked()));
  addToolbarButton(screenshotButton);
}

void OsprayViewerDialog::addViewBarButton()
{
  auto viewBarBtn = new QPushButton();
  viewBarBtn->setToolTip("Show View Options");
  viewBarBtn->setIcon(QPixmap(":/general/Resources/ViewScene/views.png"));
  connect(viewBarBtn, SIGNAL(clicked()), this, SLOT(viewBarButtonClicked()));
  addToolbarButton(viewBarBtn);
}

void OsprayViewerDialog::addControlLockButton()
{
  controlLock_ = new QPushButton();

  //TODO
  controlLock_->setDisabled(true);
}

void OsprayViewerDialog::toggleLockColor(bool locked)
{
  QString color = locked ? "red" : "rgb(66,66,69)";
  controlLock_->setStyleSheet("QPushButton { background-color: " + color + "; }");
  //autoViewButton_->setDisabled(locked);
}

void OsprayViewerDialog::autoRotateClicked()
{
#ifdef WITH_OSPRAY

#endif
}

void OsprayViewerDialog::autoViewClicked()
{
#ifdef WITH_OSPRAY
  renderer_->autoView();
#endif
}

void OsprayViewerDialog::screenshotClicked()
{
#ifdef WITH_OSPRAY

#endif
}

void OsprayViewerDialog::nextTimestepClicked()
{
#ifdef WITH_OSPRAY

#endif
}

void OsprayViewerDialog::playTimestepsClicked()
{
#ifdef WITH_OSPRAY

#endif
}

void OsprayViewerDialog::setViewportCamera()
{
#ifdef WITH_OSPRAY

#endif
}

float OsprayViewerDialog::getFloat(const Name& name) const
{
#ifdef WITH_OSPRAY
  return static_cast<float>(state_->getValue(name).toDouble());
#endif
  return 0;
}

void OsprayViewerDialog::setCameraWidgets()
{
#ifdef WITH_OSPRAY

#endif
}

void OsprayViewerDialog::setLightColor()
{
#ifdef WITH_OSPRAY

#endif
}

void OsprayViewerDialog::setBGColor()
{
#ifdef WITH_OSPRAY

#endif
}


void OsprayViewerDialog::pullSpecial()
{
  #ifdef WITH_OSPRAY
  auto ambient = colorFromState(Parameters::AmbientLightColor);
  configDialog_->ambientLightColorRDoubleSpinBox_->setValue(ambient.redF());
  configDialog_->ambientLightColorGDoubleSpinBox_->setValue(ambient.greenF());
  configDialog_->ambientLightColorBDoubleSpinBox_->setValue(ambient.blueF());

  auto directional = colorFromState(Parameters::DirectionalLightColor);
  configDialog_->directionalLightColorRDoubleSpinBox_->setValue(directional.redF());
  configDialog_->directionalLightColorGDoubleSpinBox_->setValue(directional.greenF());
  configDialog_->directionalLightColorBDoubleSpinBox_->setValue(directional.blueF());
  #endif
}

void OsprayViewerDialog::mousePositionToScreenSpace(int xIn, int yIn, float& xOut, float& yOut)
{
#ifdef WITH_OSPRAY
  int xWindow = xIn - viewer_->pos().x();
  int yWindow = yIn - viewer_->pos().y();

  xOut = (      static_cast<float>(xWindow) / renderer_->width() ) * 2.0f - 1.0f;
  yOut = (1.0 - static_cast<float>(yWindow) / renderer_->height()) * 2.0f - 1.0f;
#endif
}

MouseButton OsprayViewerDialog::getRenderButton(QMouseEvent* event)
{
#ifdef WITH_OSPRAY
  auto btn = MouseButton::NONE;
  if      (event->buttons() & Qt::LeftButton)  btn = MouseButton::LEFT;
  else if (event->buttons() & Qt::RightButton) btn = MouseButton::RIGHT;
  else if (event->buttons() & Qt::MiddleButton)   btn = MouseButton::MIDDLE;
  return btn;
#endif
  return MouseButton::NONE;
}

void OsprayViewerDialog::mousePressEvent(QMouseEvent* event)
{
  #ifdef WITH_OSPRAY
  float xSS, ySS;
  mousePositionToScreenSpace(event->x(), event->y(), xSS, ySS);

  renderer_->mousePress(xSS, ySS, getRenderButton(event));
  #endif
}

void OsprayViewerDialog::mouseMoveEvent(QMouseEvent* event)
{
  #ifdef WITH_OSPRAY
  float xSS, ySS;
  mousePositionToScreenSpace(event->x(), event->y(), xSS, ySS);

  renderer_->mouseMove(xSS, ySS, getRenderButton(event));
  #endif
}

void OsprayViewerDialog::mouseReleaseEvent(QMouseEvent* event)
{
#ifdef WITH_OSPRAY
  renderer_->mouseRelease();
  #endif
}

void OsprayViewerDialog::wheelEvent(QWheelEvent* event)
{
  #ifdef WITH_OSPRAY
  renderer_->mouseWheel(event->angleDelta().y());
  #endif
}
