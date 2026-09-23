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


#include <Interface/Modules/Render/ViewSceneVtkDialog.h>
#include <Interface/Modules/Render/ES/RendererInterfaceCollaborators.h>
#include <boost/algorithm/string/predicate.hpp>
#include <Interface/Modules/Base/CustomWidgets/CTK/ctkColorPickerButton.h>
#include <Interface/Modules/Base/CustomWidgets/CTK/ctkPopupWidget.h>

#include <Modules/Render/ViewScene.h>
#include "Modules/Render/ViewSceneVtk.h"

#include <Core/Datatypes/Feedback.h>
#include "Core/Datatypes/Color.h"
#include "Core/Logging/Log.h"
#include <Core/Application/Version.h>
#include <Core/Application/Preferences/Preferences.h>

#include <glm/glm.hpp>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Render;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Render;

#pragma GCC diagnostic ignored "-Wunused-parameter"

ViewSceneVtkManager ViewSceneVtkDialog::viewSceneManager;

ViewSceneVtkDialog::ViewSceneVtkDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent)
  : ModuleDialogGeneric(state, parent)
{
  name_ = name;
  statusBar_ = new QStatusBar(this);

  renderer_ = new VtkRenderer();
  viewer_ = new VtkQWidget(this, renderer_);

  state->connectSpecificStateChanged(Parameters::GeomData, [this]() { Q_EMIT newGeometryValueForwarder(); });
  connect(this, &ViewSceneVtkDialog::newGeometryValueForwarder, this, &ViewSceneVtkDialog::newGeometryValue);

  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  addConfigurationDialog();

  {
    toolbarHolder_ = new QMainWindow;
    toolbarHolder_->setCentralWidget(static_cast<QWidget*>(viewer_));

    toolBar1_ = new QToolBar(this);
    toolBar1_->setMovable(true);
    toolBar1_->setFloatable(true);

    toolBar2_ = new QToolBar(this);
    toolBar2_->setMovable(true);
    toolBar2_->setFloatable(true);

    toolBarController_ = new ViewSceneVtkToolBarController(this);
  }
  addToolBar();
  setToolBarPositions();
  setMinimumSize(200, 200);

  statusBar_->setMaximumHeight(20);

  vtkLayout->addWidget(toolbarHolder_);
  vtkLayout->addWidget(statusBar_);

  clippingPlaneManager_.reset(new ClippingPlaneManager(state));

  /* addCheckBoxManager(configDialog_->showPlaneCheckBox_, Parameters::ShowPlane);
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

  connect(configDialog_->viewerHeightSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &ViewSceneVtkDialog::setHeight);
  connect(configDialog_->viewerWidthSpinBox_, qOverload<int>(&QSpinBox::valueChanged), this, &ViewSceneVtkDialog::setWidth);

  connect(configDialog_->cameraViewAtXDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewAtYDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewAtZDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewFromXDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewFromYDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewFromZDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewUpXDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewUpYDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);
  connect(configDialog_->cameraViewUpZDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setViewportCamera);

  connect(configDialog_->ambientLightColorRDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setLightColor);
  connect(configDialog_->ambientLightColorGDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setLightColor);
  connect(configDialog_->ambientLightColorBDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setLightColor);
  connect(configDialog_->directionalLightColorRDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setLightColor);
  connect(configDialog_->directionalLightColorGDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setLightColor);
  connect(configDialog_->directionalLightColorBDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ViewSceneVtkDialog::setLightColor);
*/
  //float tvp[] = {-1.0f,-1.0f, 0.0f, 1.0f,-1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
  //float tvc[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  //uint32_t ind[3] = { 0, 1, 2};
}

ViewSceneVtkDialog::~ViewSceneVtkDialog()
{
  delete viewer_;
  delete renderer_;
}

void ViewSceneVtkDialog::newGeometryValue()
{
  auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
  if (!geomDataTransient || geomDataTransient->empty()) return;

  auto geom = transient_value_cast<VtkGeometryObjectHandle>(geomDataTransient);
  if (!geom) return;

  auto compGeom = std::dynamic_pointer_cast<CompositeVtkGeometryObject>(geom);

  //TODO pass geometry to the renderer_ in a renderer_ agnostic fashion
  renderer_->updateGeometries(compGeom.get()->objects());
}

void ViewSceneVtkDialog::setHeight(int h)
{
  parentWidget()->resize(width(), h);
}

void ViewSceneVtkDialog::setWidth(int w)
{
  parentWidget()->resize(w, height());
}

void ViewSceneVtkDialog::addToolBar()
{
  toolBar1_->setContextMenuPolicy(Qt::CustomContextMenu);
  WidgetStyleMixin::toolbarStyle(toolBar1_);

  toolBar2_->setOrientation(Qt::Vertical);
  WidgetStyleMixin::toolbarStyle(toolBar2_);

  //vtkLayout->addWidget(toolBar1_);
  //vtkLayout->addWidget(toolBar2_);

  addConfigurationButton();
  addAutoViewButton();
  addAutoRotateButton();
  addTimestepButtons();
  addScreenshotButton();

  addClippingPlaneButton();

  addControlLockButton();

  {
    toolBar1Position_ = new QPushButton();
    toolBar1Position_->setToolTip("Switch toolbar 1 popup direction");
    addToolbarButton(toolBar1Position_, Qt::TopToolBarArea);
  }
  {
    toolBar2Position_ = new QPushButton();
    toolBar2Position_->setToolTip("Switch toolbar 2 popup direction");
    addToolbarButton(toolBar2Position_, Qt::LeftToolBarArea);
  }
}

std::string ViewSceneVtkDialog::toString(std::string prefix) const
{
  std::string output = "VIEW_SCENE:\n";
  prefix += "  ";

  output += prefix + "State:\n";
  output += "\n";

  output += "VTK";
  output += "\n";

  return output;
}

void ViewSceneVtkDialog::adjustToolbar(double factor)
{
  if (toolBar1_)
    adjustToolbarForHighResolution(toolBar1_, factor);
  if (toolBar2_)
    adjustToolbarForHighResolution(toolBar2_, factor);
}

void ViewSceneVtkDialog::vsLog(const QString& msg) const
{
  if (statusLabel_) statusLabel_->setText(msg);
}

Qt::ToolBarArea ViewSceneVtkDialog::whereIs(QToolBar* toolbar) const
{
  if (toolbar == toolBar1_)
    return Qt::ToolBarArea::TopToolBarArea;
  else if (toolbar == toolBar2_)
    return Qt::ToolBarArea::LeftToolBarArea;
  return Qt::ToolBarArea::AllToolBarAreas;
}

void ViewSceneVtkDialog::setViewScenesToUpdate(const std::unordered_set<ViewSceneVtkDialog*>& scenes)
{
  viewScenesToUpdate.assign(scenes.begin(), scenes.end());
}

std::string ViewSceneVtkDialog::getName() const
{
  return name_;
}

void ViewSceneVtkDialog::addConfigurationButton()
{
  auto configurationButton = new QPushButton();
  configurationButton->setToolTip("Open/Close Configuration Menu");
  configurationButton->setIcon(QPixmap(":/general/Resources/ViewScene/configure.png"));
  configurationButton->setShortcut(Qt::Key_F5);
  connect(configurationButton, &QPushButton::clicked, this, &ViewSceneVtkDialog::configButtonClicked);
  addToolbarButton(configurationButton);
}

void ViewSceneVtkDialog::configButtonClicked()
{
  //configDialog_->setVisible(!configDialog_->isVisible());
}

void ViewSceneVtkDialog::addConfigurationDialog()
{
  //auto name = windowTitle() + " Configuration";
  //configDialog_ = new ViewOspraySceneConfigDialog(name, this);
}

void ViewSceneVtkDialog::addToolbarButton(QPushButton* button)
{
  button->setFixedSize(35,35);
  button->setIconSize(QSize(25,25));
  toolBar1_->addWidget(button);
}

void ViewSceneVtkDialog::addToolbarButton(QWidget* widget, Qt::ToolBarArea which, ViewSceneVtkControlPopupWidget* widgetToPopup)
{
  static const auto buttonSize = 30;
  static const auto iconSize = 22;
  widget->setFixedSize(buttonSize, buttonSize);
  auto toolbar = (which == Qt::TopToolBarArea ? toolBar1_ : toolBar2_);  // TODO refactor obviously

  if (auto* button = qobject_cast<QPushButton*>(widget))
  {
    button->setIconSize(QSize(iconSize, iconSize));
    if (widgetToPopup) setupPopupWidget(button, widgetToPopup, toolbar);
  }

  toolbar->addWidget(widget);
}

void ViewSceneVtkDialog::setupPopupWidget(QPushButton* button, ViewSceneVtkControlPopupWidget* underlyingWidget, QToolBar* toolbar)
{
  auto* popup = new ctkPopupWidget(button);
  button->setObjectName("Button: " + underlyingWidget->objectName());

  toolBarController_->setDefaultProperties(toolbar, popup);

  connect(this, &ViewSceneVtkDialog::closeAllNonPinnedPopups, [popup, underlyingWidget]() {
    if (!underlyingWidget->pinToggleAction()->isChecked()) popup->close();
  });
  connect(underlyingWidget->pinToggleAction(), &QAction::toggled, popup, &ctkPopupWidget::pinPopup);
  connect(underlyingWidget->closeAction(), &QAction::triggered, popup, &QWidget::close);

  toolBarController_->registerPopup(toolbar, popup);

  auto* popupLayout = new QVBoxLayout(popup);
  popupLayout->addWidget(underlyingWidget);
  popupLayout->setContentsMargins(4, 4, 4, 4);
}

void ViewSceneVtkDialog::addAutoViewButton()
{
  autoViewButton_ = new QPushButton(this);

  autoViewButton_->setToolTip("Auto View");
  autoViewButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  autoViewButton_->setShortcut(Qt::Key_0);
  connect(autoViewButton_, &QPushButton::clicked, this, &ViewSceneVtkDialog::autoViewClicked);
  addToolbarButton(autoViewButton_);
}

void ViewSceneVtkDialog::addAutoRotateButton()
{
  autoRotateButton_ = new QPushButton(this);
  autoRotateButton_->setToolTip("Auto Rotate");
  autoRotateButton_->setCheckable(true);
  autoRotateButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autorotate.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(autoRotateButton_, &QPushButton::clicked, this, &ViewSceneVtkDialog::autoRotateClicked);
  addToolbarButton(autoRotateButton_);
}

void ViewSceneVtkDialog::addTimestepButtons()
{
  auto nextTimestep = new QPushButton(this);
  nextTimestep->setText("Next");
  nextTimestep->setToolTip("Next timestep");
  //autoRotateButton->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(nextTimestep, &QPushButton::clicked, this, &ViewSceneVtkDialog::nextTimestepClicked);
  addToolbarButton(nextTimestep);

  playTimestepsButton_ = new QPushButton(this);
  playTimestepsButton_->setText("Play");
  playTimestepsButton_->setToolTip("Play timesteps");
  playTimestepsButton_->setCheckable(true);
  //autoRotateButton->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(playTimestepsButton_, &QPushButton::clicked, this, &ViewSceneVtkDialog::playTimestepsClicked);
  addToolbarButton(playTimestepsButton_);
}

void ViewSceneVtkDialog::addScreenshotButton()
{
  auto screenshotButton = new QPushButton(this);
  screenshotButton->setToolTip("Take screenshot");
  screenshotButton->setIcon(QPixmap(":/general/Resources/ViewScene/screenshot.png"));
  screenshotButton->setShortcut(Qt::Key_F12);
  connect(screenshotButton, &QPushButton::clicked, this, &ViewSceneVtkDialog::screenshotClicked);
  addToolbarButton(screenshotButton);
}

void ViewSceneVtkDialog::addViewBarButton()
{
  auto viewBarBtn = new QPushButton();
  viewBarBtn->setToolTip("Show View Options");
  viewBarBtn->setIcon(QPixmap(":/general/Resources/ViewScene/views.png"));
  //connect(viewBarBtn, &QPushButton::clicked, this, &ViewSceneVtkDialog::viewBarButtonClicked);
  addToolbarButton(viewBarBtn);
}

void ViewSceneVtkDialog::addControlLockButton()
{
  controlLock_ = new QPushButton();

  //TODO
  controlLock_->setDisabled(true);
}

void ViewSceneVtkDialog::addClippingPlaneButton()
{
  auto* clippingPlaneButton = new QPushButton();
  clippingPlaneButton->setIcon(QPixmap(":/general/Resources/ViewScene/clipping.png"));
  clippingPlaneControls_ = new ClippingPlaneControlsVtk(this, clippingPlaneButton);
  addToolbarButton(clippingPlaneButton, Qt::LeftToolBarArea, clippingPlaneControls_);
}

void ViewSceneVtkDialog::sendBugReport()
{
  const QString glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  const QString gpuVersion = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

  // Temporarily save screenshot so that it can be sent over email
  QString location = QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()) + ("/scirun_bug.png");
  saveScreenshot(location, false);

  // Generate email template
  const QString askForScreenshot = "\nIMPORTANT: Make sure to attach the screenshot of the ViewScene located at " % location % "\n\n\n";
  static const QString instructions =
      "## For bugs, follow the template below: fill out all pertinent sections,"
      "then delete the rest of the template to reduce clutter."
      "\n### If the prerequisite is met, just delete that text as well. "
      "If they're not all met, the issue will be closed or assigned back to you.\n\n";
  static const QString prereqs =
      "**Prerequisite**\n* [ ] Did you [perform a cursory search](https://github.com/SCIInstitute/SCIRun/issues)"
      "to see if your bug or enhancement is already reported?\n\n";
  static const QString reportGuide =
      "For more information on how to write a good "
      "[bug report](https://github.com/atom/atom/blob/master/CONTRIBUTING.md#how-do-i-submit-a-good-bug-report) or"
      "[enhancement request](https://github.com/atom/atom/blob/master/CONTRIBUTING.md#how-do-i-submit-a-good-enhancement-suggestion),"
      "see the `CONTRIBUTING` guide. These links point to another project, but most of the advice holds in general.\n\n";
  static const QString describe = "**Describe the bug**\nA clear and concise description of what the bug is.\n\n";
  static const QString askForData = "**Providing sample network(s) along with input data is useful to solving your issue.**\n\n";
  static const QString reproduction =
      "**To Reproduce**\nSteps to reproduce the behavior:"
      "\n1. Go to '...'\n2. Click on '....'\n3. Scroll down to '....'\n4. See error\n\n";

  static const QString expectedBehavior = "**Expected behavior**\nA clear and concise description of what you expected to happen.\n\n";
  static const QString additional = "**Additional context**\nAdd any other context about the problem here.\n\n";
  const QString desktopInfo = "Desktop: " % QSysInfo::prettyProductName() % "\n";
  const QString kernelInfo = "Kernel: " % QSysInfo::kernelVersion() % "\n";
  const QString gpuInfo = "GPU: " % gpuVersion % "\n";

#ifndef OLDER_QT_SUPPORT_NEEDED  // disable for older Qt 5 versions
  const QString qtInfo = "QT Version: " % QLibraryInfo::version().toString() % "\n";
  const QString glInfo = "GL Version: " % glVersion % "\n";
  const QString scirunVersionInfo = "SCIRun Version: " % QString::fromStdString(VersionInfo::GIT_VERSION_TAG) % "\n";
  const QString machineIdInfo = "Machine ID: " % QString(QSysInfo::machineUniqueId()) % "\n";

  // TODO: need generic email
  static const QString recipient = "dwhite@sci.utah.edu";
  static const QString subject = "View%20Scene%20Bug%20Report";
  QDesktopServices::openUrl(QUrl(QString("mailto:" % recipient % "?subject=" % subject % "&body=" % askForScreenshot % instructions % prereqs % reportGuide % describe % askForData % reproduction %
                                         expectedBehavior % additional % desktopInfo % kernelInfo % gpuInfo % qtInfo % glInfo % scirunVersionInfo % machineIdInfo)));
#endif
}

void ViewSceneVtkDialog::adjustZoomSpeed(int value)
{
  //auto spire = mSpire.lock();
  //spire->setZoomSpeed(value);
}

void ViewSceneVtkDialog::saveNewGeometryChanged(int state)
{
  saveScreenshotOnNewGeometry_ = state != 0;
}

void ViewSceneVtkDialog::invertZoomClicked(bool value)
{
  //auto spire = mSpire.lock();
  //spire->setZoomInverted(value);
  Preferences::Instance().invertMouseZoom.setValue(value);
}

void ViewSceneVtkDialog::menuMouseControlChanged(int index)
{
  //auto spire = mSpire.lock();
  //if (!spire) return;

  if (index == 0)
  {
    //spire->setMouseMode(MouseMode::MOUSE_OLDSCIRUN);
    Preferences::Instance().useNewViewSceneMouseControls.setValue(false);
  }
  else
  {
    //spire->setMouseMode(MouseMode::MOUSE_NEWSCIRUN);
    Preferences::Instance().useNewViewSceneMouseControls.setValue(true);
  }
  inputControls_->updateZoomOptionVisibility();
}

void ViewSceneVtkDialog::adaptToFullScreenView(bool fullScreen)
{
  isFullScreen_ = fullScreen;

  Q_EMIT fullScreenChanged();
}

void ViewSceneVtkDialog::autoRotateClicked() {}

void ViewSceneVtkDialog::resizingDone()
{
  ViewSceneFeedback vsf;
  vsf.windowSize = std::make_tuple(size().width(), size().height());
  state_->setTransientValue(Parameters::GeometryFeedbackInfo, vsf);

  state_->setValue(Parameters::WindowSizeX, size().width());
  state_->setValue(Parameters::WindowSizeY, size().height());
}

void ViewSceneVtkDialog::autoViewClicked()
{
  renderer_->autoView();
}

void ViewSceneVtkDialog::autoViewNoScaleClicked()
{
}

void ViewSceneVtkDialog::lockRotationToggled()
{
  //if (mGLWidget) mGLWidget->setLockRotation(lockRotation_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void ViewSceneVtkDialog::lockPanningToggled()
{
  //if (mGLWidget) mGLWidget->setLockPanning(lockPan_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void ViewSceneVtkDialog::lockZoomToggled()
{
  //if (mGLWidget) mGLWidget->setLockZoom(lockZoom_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void ViewSceneVtkDialog::lockAllTriggered()
{
  lockRotation_->setChecked(true);
  lockPan_->setChecked(true);
  lockZoom_->setChecked(true);
  //if (mGLWidget)
  //{
  //  mGLWidget->setLockRotation(true);
  //  mGLWidget->setLockPanning(true);
  //  mGLWidget->setLockZoom(true);
  //}
  toggleLockColor(true);
}

void ViewSceneVtkDialog::unlockAllTriggered()
{
  lockRotation_->setChecked(false);
  lockPan_->setChecked(false);
  lockZoom_->setChecked(false);
  //if (mGLWidget)
  //{
  //  mGLWidget->setLockRotation(false);
  //  mGLWidget->setLockPanning(false);
  //  mGLWidget->setLockZoom(false);
  //}
  toggleLockColor(false);
}

void ViewSceneVtkDialog::toggleAllLocks()
{
  const std::array<bool, 3> current{{lockRotation_->isChecked(), lockPan_->isChecked(), lockZoom_->isChecked()}};
  const bool anyLocked = std::any_of(current.begin(), current.end(), [](bool b) { return b; });
  if (anyLocked)
  {
    lockStateBeforeAllOff_ = current;
    unlockAllTriggered();
    return;
  }

  auto restore = lockStateBeforeAllOff_;
  if (std::none_of(restore.begin(), restore.end(), [](bool b) { return b; })) restore = {{true, true, true}};

  lockRotation_->setChecked(restore[0]);
  lockPan_->setChecked(restore[1]);
  lockZoom_->setChecked(restore[2]);
  //if (mGLWidget)
  //{
  //  mGLWidget->setLockRotation(restore[0]);
  //  mGLWidget->setLockPanning(restore[1]);
  //  mGLWidget->setLockZoom(restore[2]);
  //}
  toggleLockColor(true);
}

namespace {
QString buttonStyleSheet(bool active, const QString& activeColor = "red")
{
  QString color = active ? activeColor : "rgb(66,66,69)";
  return "QPushButton { background-color: " + color + "; }";
}
}

void ViewSceneVtkDialog::toggleLockColor(bool locked)
{
  controlLock_->setStyleSheet(buttonStyleSheet(locked));
  autoViewButton_->setDisabled(locked);
}

void ViewSceneVtkDialog::setAutoRotateSpeed(double speed)
{
  //auto spire = mSpire.lock();
  //spire->setAutoRotateSpeed(speed);
}

void ViewSceneVtkDialog::autoRotateRight()
{
  //auto spire = mSpire.lock();
  //spire->setAutoRotateVector(glm::vec2(1.0, 0.0));
  autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  //pushCameraState();
}

void ViewSceneVtkDialog::autoRotateLeft()
{
  //auto spire = mSpire.lock();
  //spire->setAutoRotateVector(glm::vec2(-1.0, 0.0));
  autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  //pushCameraState();
}

void ViewSceneVtkDialog::autoRotateUp()
{
  //auto spire = mSpire.lock();
  //spire->setAutoRotateVector(glm::vec2(0.0, 1.0));
  autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  //pushCameraState();
}

void ViewSceneVtkDialog::autoRotateDown()
{
  //auto spire = mSpire.lock();
  //spire->setAutoRotateVector(glm::vec2(0.0, -1.0));
  autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  //pushCameraState();
}

void ViewSceneVtkDialog::toggleAutoRotate()
{
  //auto spire = mSpire.lock();
  //auto currentRotate = spire->autoRotateVector();
  //if (currentRotate == glm::vec2{0, 0})
  //{
  //  spire->setAutoRotateVector(previousAutoRotate_);
  //  autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  //}
  //else
  //{
  //  previousAutoRotate_ = currentRotate;
  //  spire->setAutoRotateVector({0, 0});
  //  autoRotateButton_->setStyleSheet(buttonStyleSheet(false));
  //}

  //pushCameraState();
}

void ViewSceneVtkDialog::pullCameraRotation()
{
  //if (pushingCameraState_) return;
  //auto spire = mSpire.lock();
  //if (!spire) return;

  //glm::quat q;
  //auto rotVariable = state_->getValue(Parameters::CameraRotation);
  //if (rotVariable.value().type() == typeid(std::string))  // Legacy interpreter for networks that have this stored as string
  //  q = ViewSceneUtility::stringToQuat(state_->getValue(Parameters::CameraRotation).toString());
  //else
  //{
  //  auto rotation = toDoubleVector(rotVariable.toVector());
  //  if (rotation.size() == ViewSceneDialogImpl::QUATERNION_SIZE_)
  //    q = glm::normalize(glm::quat(rotation[0], rotation[1], rotation[2], rotation[3]));
  //  else
  //    THROW_INVALID_ARGUMENT("CameraRotation must have " + std::to_string(ViewSceneDialogImpl::QUATERNION_SIZE_) + " values. " + std::to_string(rotation.size()) + " values were provided.");
  //}

  //spire->setCameraRotation(q);
}

void ViewSceneVtkDialog::pullCameraLookAt()
{
  //if (pushingCameraState_) return;
  //auto spire = mSpire.lock();
  //if (!spire) return;

  //auto lookAtVariable = state_->getValue(Parameters::CameraLookAt);
  //if (lookAtVariable.value().type() == typeid(std::string))  // Legacy interpreter for networks that have this stored as string
  //{
  //  auto lookAtPoint = pointFromString(lookAtVariable.toString());
  //  spire->setCameraLookAt(glm::vec3(lookAtPoint[0], lookAtPoint[1], lookAtPoint[2]));
  //}
  //else
  //{
  //  auto lookAt = toDoubleVector(lookAtVariable.toVector());
  //  if (lookAt.size() == ViewSceneDialogImpl::DIMENSIONS_)
  //    spire->setCameraLookAt(glm::vec3(lookAt[0], lookAt[1], lookAt[2]));
  //  else
  //    THROW_INVALID_ARGUMENT("CameraLookAt must have " + std::to_string(ViewSceneDialogImpl::DIMENSIONS_) + " values. " + std::to_string(lookAt.size()) + " values were provided.");
  //}
}

void ViewSceneVtkDialog::pullCameraDistance()
{
  //if (pushingCameraState_) return;
  //auto spire = mSpire.lock();
  //if (!spire) return;

  //double distance = state_->getValue(Parameters::CameraDistance).toDouble();
  //double distanceMin = state_->getValue(Parameters::CameraDistanceMinimum).toDouble();
  //distance = std::max(std::abs(distance), distanceMin);
  //spire->setCameraDistance(distance);
}

void ViewSceneVtkDialog::snapToViewAxis()
{
  //auto upName = viewAxisChooser_->upVectorComboBox_->currentText();
  //if (upName.isEmpty()) return;

  //glm::vec3 up, view;
  //std::tie(view, up) = axisViewParams.at(viewAxisChooser_->currentAxis()).at(upName);

  //auto spire = mSpire.lock();
  //if (!spire) return;

  //spire->setView(view, up);

  //pushCameraState();
}

void ViewSceneVtkDialog::updateMeshComponentSelection(const QString& showFieldName, const QString& component, bool selected)
{
  auto name = showFieldName.toStdString();
  auto moduleId = name;
  auto renamed = name.find("(from ");
  if (renamed != std::string::npos) moduleId.assign(name.begin() + renamed + 6, name.end() - 1);
  MeshComponentSelectionFeedback sel(moduleId, component.toStdString(), selected);
  state_->setTransientValue(Parameters::MeshComponentSelection, sel);
}

void ViewSceneVtkDialog::setClippingPlaneIndex(int index)
{
  clippingPlaneManager_->setActive(index);

  doClippingPlanes();
}

void ViewSceneVtkDialog::setClippingPlaneVisible(bool value)
{
  clippingPlaneManager_->setActiveVisibility(value);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::setClippingPlaneFrameOn(bool value)
{
  // updateModifiedGeometries();
  clippingPlaneManager_->setActiveFrameOn(value);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::reverseClippingPlaneNormal(bool value)
{
  clippingPlaneManager_->setActiveNormalReversed(value);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::setClippingPlaneX(int index)
{
  clippingPlaneManager_->setActiveX(index);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::setClippingPlaneY(int index)
{
  clippingPlaneManager_->setActiveY(index);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::setClippingPlaneZ(int index)
{
  clippingPlaneManager_->setActiveZ(index);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::setClippingPlaneD(int index)
{
  clippingPlaneManager_->setActiveD(index);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::showOrientationChecked(bool value)
{
  //auto spire = mSpire.lock();
  //spire->showOrientation(value);
  state_->setValue(Parameters::AxesVisible, value);
}

void ViewSceneVtkDialog::setOrientAxisSize(int value)
{
  //auto spire = mSpire.lock();
  //spire->setOrientSize(value);
  state_->setValue(Parameters::AxesSize, value);
}

void ViewSceneVtkDialog::setOrientAxisPosX(int pos)
{
  //auto spire = mSpire.lock();
  //spire->setOrientPosX(pos);
  state_->setValue(Parameters::AxesX, pos);
}

void ViewSceneVtkDialog::setOrientAxisPosY(int pos)
{
  //auto spire = mSpire.lock();
  //spire->setOrientPosY(pos);
  state_->setValue(Parameters::AxesY, pos);
}

void ViewSceneVtkDialog::setCenterOrientPos()
{
  setOrientAxisPosX(50);
  setOrientAxisPosY(50);
}

void ViewSceneVtkDialog::setDefaultOrientPos()
{
  setOrientAxisPosX(100);
  setOrientAxisPosY(100);
}

void ViewSceneVtkDialog::setScaleBarVisible(bool value)
{
  scaleBar_.visible = value;
  state_->setValue(Parameters::ShowScaleBar, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarFontSize(int value)
{
  scaleBar_.fontSize = value;
  state_->setValue(Parameters::ScaleBarFontSize, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarUnitValue(const QString& text)
{
  scaleBar_.unit = text.toStdString();
  state_->setValue(Parameters::ScaleBarUnitValue, text.toStdString());
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarLength(double value)
{
  scaleBar_.length = value;
  state_->setValue(Parameters::ScaleBarLength, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarHeight(double value)
{
  scaleBar_.height = value;
  state_->setValue(Parameters::ScaleBarHeight, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarMultiplier(double value)
{
  scaleBar_.multiplier = value;
  state_->setValue(Parameters::ScaleBarMultiplier, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarNumTicks(int value)
{
  scaleBar_.numTicks = value;
  state_->setValue(Parameters::ScaleBarNumTicks, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarLineColor(double value)
{
  scaleBar_.lineColor = value;
  state_->setValue(Parameters::ScaleBarLineColor, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBarLineWidth(double value)
{
  scaleBar_.lineWidth = value;
  state_->setValue(Parameters::ScaleBarLineWidth, value);
  setScaleBar();
}

void ViewSceneVtkDialog::setScaleBar()
{
  if (scaleBar_.visible)
  {
    //updateScaleBarLength();
    //scaleBarGeom_ = buildGeometryScaleBar();
  }
  //updateModifiedGeometries();
}

namespace {
float toInclination(int value)
{
  return value / 180.0f * M_PI - M_PI / 2.0f;
}

float toAzimuth(int value)
{
  return value / 180.0f * M_PI - M_PI;
}

static const std::vector<AlgorithmParameterName> lightColorKeys = {Parameters::HeadLightColor, Parameters::Light1Color, Parameters::Light2Color, Parameters::Light3Color};
static const std::vector<AlgorithmParameterName> lightInclinationKeys = {Parameters::HeadLightInclination, Parameters::Light1Inclination, Parameters::Light2Inclination, Parameters::Light3Inclination};
static const std::vector<AlgorithmParameterName> lightAzimuthKeys = {Parameters::HeadLightAzimuth, Parameters::Light1Azimuth, Parameters::Light2Azimuth, Parameters::Light3Azimuth};
static const std::vector<AlgorithmParameterName> lightOnKeys = {Parameters::HeadLightOn, Parameters::Light1On, Parameters::Light2On, Parameters::Light3On};

}

void ViewSceneVtkDialog::setLightColor(int index)
{
  const auto lightColor(lightControls_[index]->color());

  state_->setValue(lightColorKeys[index], ColorRGB(lightColor.redF(), lightColor.greenF(), lightColor.blueF()).toString());

  //auto spire = mSpire.lock();
  //if (spire) spire->setLightColor(index, lightColor.redF(), lightColor.greenF(), lightColor.blueF());
}

void ViewSceneVtkDialog::setLightInclination(int index, int value)
{
  state_->setValue(lightInclinationKeys[index], value);
  //auto spire = mSpire.lock();
  //spire->setLightInclination(index, toInclination(value));
}

void ViewSceneVtkDialog::toggleLight(int index, bool value)
{
  state_->setValue(lightOnKeys[index], value);
  //auto spire = mSpire.lock();
  //if (spire) spire->setLightOn(index, value);
}

void ViewSceneVtkDialog::setLightAzimuth(int index, int value)
{
  state_->setValue(lightAzimuthKeys[index], value);
  //auto spire = mSpire.lock();
  //spire->setLightAzimuth(index, toAzimuth(value));
}

void ViewSceneVtkDialog::toggleAllLights()
{
  std::array<bool, ViewSceneVtkDialog::NUM_LIGHTS> current{};
  for (size_t i = 0; i < current.size(); ++i)
    current[i] = lightControls_[i]->isLightOn();

  if (std::any_of(current.begin(), current.end(), [](bool b) { return b; }))
  {
    lightStateBeforeAllOff_ = current;
    for (auto* light : lightControls_)
      light->setLightOn(false);
    return;
  }

  auto restore = lightStateBeforeAllOff_;
  // Everything off at startup, or saved from an all-off state: fall back to the
  // headlight rather than switching on lights the user never turned on.
  if (std::none_of(restore.begin(), restore.end(), [](bool b) { return b; })) restore[0] = true;

  for (size_t i = 0; i < restore.size(); ++i)
    lightControls_[i]->setLightOn(restore[i]);
}

void ViewSceneVtkDialog::setAmbientValue(double value)
{
  state_->setValue(Parameters::Ambient, value);
  setMaterialFactor(MatFactor::MAT_AMBIENT, value);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setDiffuseValue(double value)
{
  state_->setValue(Parameters::Diffuse, value);
  setMaterialFactor(MatFactor::MAT_DIFFUSE, value);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setSpecularValue(double value)
{
  state_->setValue(Parameters::Specular, value);
  setMaterialFactor(MatFactor::MAT_SPECULAR, value);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setShininessValue(double value)
{
  const static int maxSpecExp = 40;
  const static int minSpecExp = 1;
  state_->setValue(Parameters::Shine, value);
  // taking square of value makes the ui a little more intuitive in my opinion
  setMaterialFactor(MatFactor::MAT_SHINE, value * value * (maxSpecExp - minSpecExp) + minSpecExp);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setFogOn(bool value)
{
  state_->setValue(Parameters::FogOn, value);
  setFog(FogFactor::FOG_INTENSITY, value ? 1.0 : 0.0);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setFogUseBGColor(bool value)
{
  state_->setValue(Parameters::UseBGColor, value);
  if (value)
    setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
  else
  {
    auto fogColor = fogControls_->color();
    setFogColor(glm::vec4(fogColor.red(), fogColor.green(), fogColor.blue(), 1.0));
  }
  //updateAllGeometries();
}

void ViewSceneVtkDialog::assignFogColor()
{
  auto fogColor = fogControls_->color();
  state_->setValue(Parameters::FogColor, ColorRGB(fogColor.red(), fogColor.green(), fogColor.blue()).toString());
  bool useBg = state_->getValue(Parameters::UseBGColor).toBool();
  if (!useBg)
  {
    setFogColor(glm::vec4(fogColor.red(), fogColor.green(), fogColor.blue(), 1.0));
    //updateAllGeometries();
  }
}

void ViewSceneVtkDialog::setFogStartValue(double value)
{
  state_->setValue(Parameters::FogStart, value);
  setFog(FogFactor::FOG_START, value);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setFogEndValue(double value)
{
  state_->setValue(Parameters::FogEnd, value);
  setFog(FogFactor::FOG_END, value);
  //updateAllGeometries();
}

namespace {
// The table spells modifiers the Qt way ("Ctrl+0", "Alt+H"). On macOS Qt maps
// those to Command and Option, so show the glyphs Mac users expect.
QString platformShortcutDisplay(const char* display)
{
  QString text(display);
#ifdef __APPLE__
  text.replace("Ctrl+", QString(QChar(0x2318)));  // Command
  text.replace("Alt+", QString(QChar(0x2325)));   // Option
#endif
  return text;
}
}

void ViewSceneVtkDialog::showShortcutsDialog()
{
  if (!shortcutsDialog_)
  {
    shortcutsDialog_ = new QDialog(this);
    shortcutsDialog_->installEventFilter(this);
    Ui::ViewSceneShortcuts shortcutsUi;
    shortcutsUi.setupUi(shortcutsDialog_);
    auto* table = shortcutsUi.tableWidget;
    shortcutsTable_ = table;
    table->installEventFilter(this);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Only show implemented shortcuts; unimplemented ones are hidden pending
    // their own feature work (see TODO comments in shortcutTable()).
    int row = 0;
    for (int idx = 0; idx < static_cast<int>(numShortcuts); ++idx)
    {
      const auto& sc = shortcutTable()[static_cast<std::size_t>(idx)];
      if (!sc.isImplemented()) continue;
      table->insertRow(row);
      auto* nameItem = new QTableWidgetItem(sc.actionName);
      auto* shortcutItem = new QTableWidgetItem(platformShortcutDisplay(sc.shortcutDisplay));
      auto* descItem = new QTableWidgetItem(sc.description);
      // Store the original table index so the double-click handler can find it.
      nameItem->setData(Qt::UserRole, idx);
      table->setItem(row, 0, nameItem);
      table->setItem(row, 1, shortcutItem);
      table->setItem(row, 2, descItem);
      ++row;
    }
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    // Let the table report its exact content size so the dialog can fit it.
    table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    table->setToolTip("Double-click a row to perform that action");
    connect(table, &QTableWidget::cellDoubleClicked, [this](int row, int /*col*/) {
      const auto* nameItem = shortcutsTable_->item(row, 0);
      if (!nameItem) return;
      const int idx = nameItem->data(Qt::UserRole).toInt();
      const auto& sc = shortcutTable()[static_cast<std::size_t>(idx)];
      if (sc.action) sc.action(this);
    });
    // Size the dialog to fit the content, plus padding so the last row is
    // fully visible without scrolling (Windows chrome also needs extra room).
    shortcutsDialog_->adjustSize();
    const QSize padded = shortcutsDialog_->size() + QSize(40, 60);
    shortcutsDialog_->setFixedSize(padded);

    // Default position: top-right of the ViewScene window to minimise overlap.
    // Use saved position if the user has moved it previously this session.
    const QPoint defaultPos = mapToGlobal(QPoint(width(), 0));
    shortcutsDialog_->move(shortcutsDialogPos_.value_or(defaultPos));
  }
  shortcutsDialog_->show();
  shortcutsDialog_->raise();
  shortcutsDialog_->activateWindow();
}

void ViewSceneVtkDialog::assignBackgroundColor()
{
  const auto title = windowTitle() + " Choose background color";
  const auto newColor = QColorDialog::getColor(bgColor_, this, title);
  if (newColor.isValid())
  {
    bgColor_ = newColor;
    colorOptions_->setSampleColor(bgColor_);
    state_->setValue(Parameters::BackgroundColor, ColorRGB(bgColor_.red(), bgColor_.green(), bgColor_.blue()).toString());
    //auto spire = mSpire.lock();
    //spire->setBackgroundColor(bgColor_);
    const auto useBg = state_->getValue(Parameters::UseBGColor).toBool();
    if (useBg)
      setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
    else
    {
      const auto fogColor = fogControls_->color();
      setFogColor(glm::vec4(fogColor.red(), fogColor.green(), fogColor.blue(), 1.0));
    }
    //updateAllGeometries();
  }
}

void ViewSceneVtkDialog::setTransparencySortTypeContinuous(bool)
{
  //auto spire = mSpire.lock();
  //spire->setTransparencyRenderType(RenderState::TransparencySortType::CONTINUOUS_SORT);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setTransparencySortTypeUpdate(bool)
{
  //auto spire = mSpire.lock();
  //spire->setTransparencyRenderType(RenderState::TransparencySortType::UPDATE_SORT);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::setTransparencySortTypeLists(bool)
{
  //auto spire = mSpire.lock();
  //spire->setTransparencyRenderType(RenderState::TransparencySortType::LISTS_SORT);
  //updateAllGeometries();
}

void ViewSceneVtkDialog::screenshotSaveAs()
{
  //auto fileName = QFileDialog::getSaveFileName(mGLWidget, "Save screenshot...", QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()), "*.png");

  //saveScreenshot(fileName, true);
}

void ViewSceneVtkDialog::quickScreenshot()
{
  auto fileName = QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()) +
                  QString("/%1_%2.png").arg(QString::fromStdString(getName()).replace(':', '-')).arg(QDateTime::currentDateTime().toString("yyyy.MM.dd.HHmmss.zzz"));

  saveScreenshot(fileName, true);
}

void ViewSceneVtkDialog::setScreenshotDirectory()
{
  auto dir = QFileDialog::getExistingDirectory(this, tr("Choose Screenshot Directory"), QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()));

  state_->setValue(Parameters::ScreenshotDirectory, dir.toStdString());
}

void ViewSceneVtkDialog::setToolBarPositions()
{
  auto toolBar1Position = static_cast<Qt::ToolBarArea>(state_->getValue(Parameters::ToolBarMainPosition).toInt());
  auto toolBar2Position = static_cast<Qt::ToolBarArea>(state_->getValue(Parameters::ToolBarRenderPosition).toInt());
  auto toolBar3Position = static_cast<Qt::ToolBarArea>(state_->getValue(Parameters::ToolBarAdvancedPosition).toInt());
  toolbarHolder_->addToolBar(toolBar1Position, toolBar1_);
  toolbarHolder_->addToolBar(toolBar2Position, toolBar2_);
  connect(toolBar1_, &QToolBar::topLevelChanged, [this](bool /*topLevel*/) { state_->setValue(Parameters::ToolBarMainPosition, static_cast<int>(whereIs(toolBar1_))); });
  connect(toolBar2_, &QToolBar::topLevelChanged, [this](bool /*topLevel*/) { state_->setValue(Parameters::ToolBarRenderPosition, static_cast<int>(whereIs(toolBar2_))); });

  toolBarController_->registerDirectionButton(toolBar1_, toolBar1Position_);
  toolBarController_->registerDirectionButton(toolBar2_, toolBar2Position_);
}

void ViewSceneVtkDialog::screenshotClicked() {}

void ViewSceneVtkDialog::nextTimestepClicked()
{
}

void ViewSceneVtkDialog::playTimestepsClicked()
{
}

void ViewSceneVtkDialog::setViewportCamera()
{
}

float ViewSceneVtkDialog::getFloat(const Name& name) const
{
  return static_cast<float>(state_->getValue(name).toDouble());
}

using V = glm::vec3;

void ViewSceneVtkDialog::setAxisView(int n)
{
  // Maps key 1-6 to the six cardinal axis-aligned views with sensible up vectors.
  static const std::pair<glm::vec3, glm::vec3> views[6] = {
      {V(1, 0, 0), V(0, 1, 0)},   // 1: +X  (Y up)
      {V(-1, 0, 0), V(0, 1, 0)},  // 2: -X  (Y up)
      {V(0, 1, 0), V(0, 0, 1)},   // 3: +Y  (Z up)
      {V(0, -1, 0), V(0, 0, 1)},  // 4: -Y  (Z up)
      {V(0, 0, 1), V(0, 1, 0)},   // 5: +Z  (Y up)
      {V(0, 0, -1), V(0, 1, 0)},  // 6: -Z  (Y up)
  };
  if (n < 1 || n > 6) return;
  //auto spire = mSpire.lock();
  //if (!spire) return;
  //spire->setView(views[n - 1].first, views[n - 1].second);
  //pushCameraState();
}

void ViewSceneVtkDialog::setClosestAxisView()
{
  //auto spire = mSpire.lock();
  //if (!spire) return;

  // The camera rotation quaternion is from glm::lookAt (world→camera).
  // Transposing its mat3 gives the camera axes in world space:
  //   Rt[0] = right,  Rt[1] = up,  Rt[2] = -forward  (all in world coords)
  const glm::mat3 Rt;// = glm::transpose(glm::mat3_cast(spire->getCameraRotation()));
  const glm::vec3 viewDir = -Rt[2];  // current look direction (world space)
  const glm::vec3 upDir = Rt[1];     // current up direction  (world space)

  static const glm::vec3 axes[6] = {
      {1, 0, 0},
      {-1, 0, 0},
      {0, 1, 0},
      {0, -1, 0},
      {0, 0, 1},
      {0, 0, -1},
  };

  // Find the cardinal axis closest to the current view direction.
  int bestView = 0;
  float bestViewDot = -2.f;
  for (int i = 0; i < 6; ++i)
  {
    float d = glm::dot(viewDir, axes[i]);
    if (d > bestViewDot)
    {
      bestViewDot = d;
      bestView = i;
    }
  }
  const glm::vec3 view = axes[bestView];

  // Find the cardinal axis closest to the current up direction that is
  // perpendicular to the chosen view axis.
  int bestUp = 0;
  float bestUpDot = -2.f;
  for (int i = 0; i < 6; ++i)
  {
    if (std::abs(glm::dot(view, axes[i])) > 0.5f) continue;  // skip parallel/anti-parallel
    float d = glm::dot(upDir, axes[i]);
    if (d > bestUpDot)
    {
      bestUpDot = d;
      bestUp = i;
    }
  }

  //spire->setView(view, axes[bestUp]);
  //pushCameraState();
}

void ViewSceneVtkDialog::setCameraWidgets() {}

void ViewSceneVtkDialog::setLightColor()
{
}

void ViewSceneVtkDialog::setBGColor()
{
}

void ViewSceneVtkDialog::setMaterialFactor(MatFactor factor, double value)
{
  //auto spire = mSpire.lock();
  //if (spire) spire->setMaterialFactor(factor, value);
}

void ViewSceneVtkDialog::setFog(FogFactor factor, double value)
{
  //auto spire = mSpire.lock();
  //if (spire) spire->setFog(factor, value);
}

void ViewSceneVtkDialog::setFogColor(const glm::vec4& color)
{
  //auto spire = mSpire.lock();
  //if (spire) spire->setFogColor(color / 255.0f);
}

void ViewSceneVtkDialog::saveScreenshot(QString fileName, bool notify)
{
  //if (!fileName.isEmpty())
  //{
  //  takeScreenshot();
  //  if (notify) QMessageBox::information(nullptr, "ViewScene Screenshot", "Saving ViewScene screenshot to: " + fileName);

  //  screenshotTaker_->saveScreenshot(fileName);
  //}
}

const ViewSceneVtkDialog::ShortcutTable& ViewSceneVtkDialog::shortcutTable()
{
  using Id = ShortcutDef::Id;
  // Lambdas take ViewSceneDialog* so they can call any slot on the instance.
  // Protected access is fine here because this is a member function of ViewSceneDialog.
  static const ShortcutTable table = {{
      {Id::AxisViews, Qt::Key_1, Qt::NoModifier, "Axis Views", "1-6", "Preprogrammed views aligning the view with the X, Y, or Z-axis", [](ViewSceneVtkDialog* d) { d->setAxisView(1); }},
      {Id::Autoview, Qt::Key_0, Qt::NoModifier, "Autoview", "0", "Find a view that shows all the data", [](ViewSceneVtkDialog* d) { d->autoViewClicked(); }},
      // Qt maps Qt::ControlModifier to Command on macOS, which is what we want here:
      // unlike Cmd+H (hide window) there is no system conflict on Cmd+0. The main
      // window does use Ctrl+0 for "Reset Network Zoom"; event() claims the key back
      // while this dialog has focus.
      {Id::AutoviewNoScale, Qt::Key_0, Qt::ControlModifier, "Autoview (no scale)", "Ctrl+0", "Reset the eye so the data is centered", [](ViewSceneVtkDialog* d) { d->autoViewNoScaleClicked(); }},
      {Id::SnapToAxis, Qt::Key_X, Qt::NoModifier, "Snap to Axis", "X", "Snap to the nearest axis-aligned view", [](ViewSceneVtkDialog* d) { d->setClosestAxisView(); }},
      // TODO(#2510): Copy View (Ctrl+1-9) — enumerate all live ViewSceneDialog instances via
      // ViewSceneManager, let user pick by index, then call spire->setCameraDistance/
      // setCameraLookAt/setCameraRotation with values from the chosen window's spire.
      // Blocked on: ViewSceneManager exposing an ordered list of active ViewScenes.
      {Id::CopyView, Qt::Key_1, Qt::ControlModifier, "Copy View", "Ctrl+1-9", "Copy view from Viewer Window 1-9", nullptr},
      // Set Home stays on Alt (Option on macOS) on every platform: Qt maps
      // Qt::ControlModifier to Command, and Cmd+H is hide-window on macOS.
      {Id::SetHome, Qt::Key_H, Qt::AltModifier, "Set Home", "Alt+H", "Store the current view",
          [](ViewSceneVtkDialog* d) {
            //auto spire = d->mSpire.lock();
            //if (!spire) return;
            //d->homeView_ = ViewSceneDialogImpl::HomeCamera{spire->getCameraDistance(), spire->getCameraLookAt(), spire->getCameraRotation()};
          }},
      {Id::GotoHome, Qt::Key_H, Qt::NoModifier, "Home", "H", "Go back to the stored view",
          [](ViewSceneVtkDialog* d) {
            //if (!d->homeView_) return;
            //auto spire = d->mSpire.lock();
            //if (!spire) return;
            //const auto& hv = *d->homeView_;
            //spire->setCameraDistance(hv.distance);
            //spire->setCameraLookAt(hv.lookAt);
            //spire->setCameraRotation(hv.rotation);
            //d->pushCameraState();
          }},
      // TODO(#2503): Toggle World Axes (A) — v4 rendered a large XYZ triad at the true world
      // origin. v5 has no equivalent. Needs: a new GeometryObject that draws axis lines
      // (or glyphs) at (0,0,0), a Parameters::WorldAxesVisible state entry, and wiring
      // through newGeometryValue. The corner orientation icon (O) is a separate feature.
      {Id::ToggleAxes, Qt::Key_A, Qt::NoModifier, "Toggle Axes", "A", "Switch axes on/off", nullptr},
      // TODO(#2504): Bounding Box (B) — Parameters::ShowBBox exists but is commented out throughout
      // the renderer. Needs: re-enabling ShowBBox, computing the combined scene AABB in
      // SRInterface, building a wire-frame box GeometryObject each frame it's on, and
      // a toggleBoundingBox() slot here similar to showOrientationChecked().
      {Id::BoundingBox, Qt::Key_B, Qt::NoModifier, "Bounding Box", "B", "Switch bounding box mode on/off", nullptr},
      // The toggles below drive their control-dock widget rather than the dialog slot
      // the widget is connected to. The slots are downstream receivers: calling one
      // directly leaves the check box -- and the toolbar button styled from it -- stale.
      {Id::ToggleClipping, Qt::Key_C, Qt::NoModifier, "Toggle Clipping", "C", "Switch clipping on/off", [](ViewSceneVtkDialog* d) { d->clippingPlaneControls_->toggleVisible(); }},
      {Id::ToggleFog, Qt::Key_D, Qt::NoModifier, "Toggle Fog", "D", "Switch fog on/off",
          // fogGroupBox_ is connected on clicked(), which setChecked() does not emit,
          // so toggleFog() re-emits it for us.
          [](ViewSceneVtkDialog* d) { d->fogControls_->toggleFog(); }},
      // TODO(#2505): Flat Shading (F) — no flat-shading mode in the v5 renderer. Needs: a uniform
      // flag in the object/phong shaders to use face normals (or a flat-shading shader
      // variant), a StaticRenderMode or per-pass uniform, SRInterface::setFlatShading(bool),
      // and a Parameters::FlatShading state entry with a toggleFlatShading() slot.
      {Id::FlatShading, Qt::Key_F, Qt::NoModifier, "Flat Shading", "F", "Switch flat shading on/off", nullptr},
      {Id::OpenHelp, Qt::Key_I, Qt::NoModifier, "Open Help", "I", "Open this help window", [](ViewSceneVtkDialog* d) { d->showShortcutsDialog(); }},
      {Id::ViewLocking, Qt::Key_L, Qt::NoModifier, "View Locking", "L", "Switch view locking on/off", [](ViewSceneVtkDialog* d) { d->toggleAllLocks(); }},
      {Id::ToggleLighting, Qt::Key_K, Qt::NoModifier, "Toggle Lighting", "K", "Switch lighting on/off", [](ViewSceneVtkDialog* d) { d->toggleAllLights(); }},
      {Id::OrientationIcon, Qt::Key_O, Qt::NoModifier, "Orientation Icon", "O", "Switch orientation icon on/off",
          [](ViewSceneVtkDialog* d) { d->orientationAxesControls_->toggleOrientation(); }},
      // TODO(#2506): Orthographic (P) — SRCamera/SRInterface only expose perspective projection.
      // Needs: SRInterface::setOrthographic(bool) that swaps between glm::perspective and
      // a glm::ortho sized to the current view frustum width at the lookAt distance,
      // SRCamera::setAsPerspective already exists — add setAsOrthographic() alongside it,
      // and a Parameters::OrthographicMode state entry with a toggleOrthographic() slot.
      {Id::Orthographic, Qt::Key_P, Qt::NoModifier, "Orthographic", "P", "Switch orthographic projection on/off", nullptr},
      // TODO(#2507): Stereo (S) — not implemented in v5. Needs: a stereo rendering mode in
      // SRInterface (side-by-side or anaglyph), likely a second render pass with a
      // laterally offset camera, SRInterface::setStereo(bool), and a
      // Parameters::StereoMode state entry. Significant renderer work.
      {Id::Stereo, Qt::Key_S, Qt::NoModifier, "Stereo", "S", "Switch stereo mode on/off", nullptr},
      // TODO(#2508): Backculling (U) — no back-face cull toggle in v5. Needs: SRInterface::
      // setBackfaceCulling(bool) that calls glEnable/glDisable(GL_CULL_FACE) + glCullFace
      // (GL_BACK) in the render loop (or a StaticGLState flag), a Parameters::BackfaceCulling
      // state entry, and a toggleBackfaceCulling() slot.
      {Id::Backculling, Qt::Key_U, Qt::NoModifier, "Backculling", "U", "Switch backculling on/off", nullptr},
      // TODO(#2509): Wireframe (W) — no wireframe mode in v5. Needs: SRInterface::setWireframe(bool)
      // using glPolygonMode(GL_FRONT_AND_BACK, GL_LINE/GL_FILL) (desktop GL only; for ES
      // compatibility a geometry-shader or line-drawing pass may be needed instead),
      // a Parameters::WireframeMode state entry, and a toggleWireframe() slot.
      {Id::Wireframe, Qt::Key_W, Qt::NoModifier, "Wireframe", "W", "Switch wire frame on/off", nullptr},
  }};
  return table;
}

void ViewSceneVtkDialog::pullSpecial()
{
  if (!pulledSavedVisibility_)
  {
    //pullCameraState();
    //const auto show = state_->getValue(Parameters::ShowViewer).toBool();
    //if (show && parentWidget())
    //{
    //  parentWidget()->show();
    //}

    //adjustSizeFromState();

    //if (parentWidget())
    //{
    //  auto dock = qobject_cast<QDockWidget*>(parentWidget());
    //  const auto isFloating = state_->getValue(Parameters::IsFloating).toBool();
    //  if (dock) dock->setFloating(isFloating);

    //  if (isFloating)
    //  {
    //    if (savedPos_)
    //    {
    //      parentWidget()->move(*savedPos_);
    //    }
    //    else
    //    {
    //      adjustPositionFromState();
    //    }
    //  }
    //}
    //clippingPlaneManager_->loadFromState();
    initializeClippingPlaneDisplay();
    //initializeAxes();
    //initializeVisibleObjects();
    //setInitialLightValues();
    pulledSavedVisibility_ = true;
  }
}

void ViewSceneVtkDialog::mousePositionToScreenSpace(int xIn, int yIn, float& xOut, float& yOut)
{
  int xWindow = xIn - viewer_->pos().x();
  int yWindow = yIn - viewer_->pos().y();

  xOut = (      static_cast<float>(xWindow) / renderer_->width() ) * 2.0f - 1.0f;
  yOut = (1.0 - static_cast<float>(yWindow) / renderer_->height()) * 2.0f - 1.0f;
}

MouseButton ViewSceneVtkDialog::getRenderButton(QMouseEvent* event)
{
  auto btn = MouseButton::NONE;
  if      (event->buttons() & Qt::LeftButton)  btn = MouseButton::LEFT;
  else if (event->buttons() & Qt::RightButton) btn = MouseButton::RIGHT;
  else if (event->buttons() & Qt::MiddleButton)   btn = MouseButton::MIDDLE;
  return btn;
}

void ViewSceneVtkDialog::mousePressEvent(QMouseEvent* event)
{
  const float x = static_cast<float>(event->x() - viewer_->pos().x());

  const float y = static_cast<float>(event->y() - viewer_->pos().y());

  renderer_->mousePress(x, y, getRenderButton(event));
}

void ViewSceneVtkDialog::mouseMoveEvent(QMouseEvent* event)
{
  const float x = static_cast<float>(event->x() - viewer_->pos().x());

  const float y = static_cast<float>(event->y() - viewer_->pos().y());

  renderer_->mouseMove(x, y, getRenderButton(event));
}

void ViewSceneVtkDialog::mouseReleaseEvent(QMouseEvent* event)
{
  renderer_->mouseRelease();
}

void ViewSceneVtkDialog::wheelEvent(QWheelEvent* event)
{
  renderer_->mouseWheel(event->angleDelta().y());
}

void ViewSceneVtkDialog::initializeClippingPlaneDisplay()
{
  clippingPlaneManager_->setActive(0);

  const auto& activePlane = clippingPlaneManager_->active();
  clippingPlaneControls_->updatePlaneSettingsDisplay(activePlane.visible, activePlane.showFrame, activePlane.reverseNormal);
  clippingPlaneControls_->updatePlaneControlDisplay(activePlane.x, activePlane.y, activePlane.z, activePlane.d);
}

void ViewSceneVtkDialog::doClippingPlanes()
{
  const auto& activePlane = clippingPlaneManager_->active();
  clippingPlaneControls_->updatePlaneSettingsDisplay(activePlane.visible, activePlane.showFrame, activePlane.reverseNormal);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::updateClippingPlaneDisplay()
{
  renderer_->updateClippingPlanes(clippingPlaneManager_->allPlanes());
  const auto& activePlane = clippingPlaneManager_->active();
  clippingPlaneControls_->updatePlaneControlDisplay(activePlane.x, activePlane.y, activePlane.z, activePlane.d);

}
