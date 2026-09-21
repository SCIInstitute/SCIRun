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

#ifdef WITH_VTK
//#include <ospray/ospray.h>

#include <Modules/Render/ViewScene.h>
#include "Modules/Render/ViewSceneVtk.h"
//#include "Interface/Modules/Render/Ospray/QOSPRayWidget.h"
//#include "Interface/Modules/Render/Ospray/OSPRayRenderer.h"
//#include "Interface/Modules/Render/ViewOspraySceneConfig.h"

#include <Core/Datatypes/Feedback.h>
#include "Core/Datatypes/Color.h"
#include "Core/Logging/Log.h"
#endif

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
  #ifdef WITH_VTK
using namespace SCIRun::Core::Algorithms::Render;
#endif
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Render;

#pragma GCC diagnostic ignored "-Wunused-parameter"

ViewSceneVtkDialog::ViewSceneVtkDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent)
  : ModuleDialogGeneric(state, parent)
{
  #ifdef WITH_VTK
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

  #endif
}

ViewSceneVtkDialog::~ViewSceneVtkDialog()
{
#ifdef WITH_VTK
  delete viewer_;
  delete renderer_;
#endif
}

void ViewSceneVtkDialog::newGeometryValue()
{
#ifdef WITH_VTK

  auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
  if (!geomDataTransient || geomDataTransient->empty()) return;

  auto geom = transient_value_cast<VtkGeometryObjectHandle>(geomDataTransient);
  if (!geom) return;

  auto compGeom = std::dynamic_pointer_cast<CompositeVtkGeometryObject>(geom);

  //TODO pass geometry to the renderer_ in a renderer_ agnostic fashion
  renderer_->updateGeometries(compGeom.get()->objects());
#endif
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

void ViewSceneVtkDialog::adjustToolbar(double factor)
{
  if (toolBar1_)
    adjustToolbarForHighResolution(toolBar1_, factor);
  if (toolBar2_)
    adjustToolbarForHighResolution(toolBar2_, factor);
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

Qt::ToolBarArea ViewSceneVtkDialog::whereIs(QToolBar* toolbar) const
{
  if (toolbar == toolBar1_)
    return Qt::ToolBarArea::TopToolBarArea;
  else if (toolbar == toolBar2_)
    return Qt::ToolBarArea::LeftToolBarArea;
  return Qt::ToolBarArea::AllToolBarAreas;
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
#ifdef WITH_VTK
  //configDialog_->setVisible(!configDialog_->isVisible());
#endif
}

void ViewSceneVtkDialog::addConfigurationDialog()
{
#ifdef WITH_VTK
  //auto name = windowTitle() + " Configuration";
  //configDialog_ = new ViewOspraySceneConfigDialog(name, this);
#endif
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

void ViewSceneVtkDialog::toggleLockColor(bool locked)
{
  QString color = locked ? "red" : "rgb(66,66,69)";
  controlLock_->setStyleSheet("QPushButton { background-color: " + color + "; }");
  //autoViewButton_->setDisabled(locked);
}

void ViewSceneVtkDialog::autoRotateClicked()
{
#ifdef WITH_VTK

#endif
}

void ViewSceneVtkDialog::autoViewClicked()
{
#ifdef WITH_VTK
  renderer_->autoView();
#endif
}

void ViewSceneVtkDialog::screenshotClicked()
{
#ifdef WITH_VTK

#endif
}

void ViewSceneVtkDialog::nextTimestepClicked()
{
#ifdef WITH_VTK

#endif
}

void ViewSceneVtkDialog::playTimestepsClicked()
{
#ifdef WITH_VTK

#endif
}

void ViewSceneVtkDialog::setViewportCamera()
{
#ifdef WITH_VTK

#endif
}

float ViewSceneVtkDialog::getFloat(const Name& name) const
{
#ifdef WITH_VTK
  return static_cast<float>(state_->getValue(name).toDouble());
#endif
  return 0;
}

void ViewSceneVtkDialog::setCameraWidgets()
{
#ifdef WITH_VTK

#endif
}

void ViewSceneVtkDialog::setLightColor()
{
#ifdef WITH_VTK

#endif
}

void ViewSceneVtkDialog::setBGColor()
{
#ifdef WITH_VTK

#endif
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
#ifdef WITH_VTK
  int xWindow = xIn - viewer_->pos().x();
  int yWindow = yIn - viewer_->pos().y();

  xOut = (      static_cast<float>(xWindow) / renderer_->width() ) * 2.0f - 1.0f;
  yOut = (1.0 - static_cast<float>(yWindow) / renderer_->height()) * 2.0f - 1.0f;
#endif
}

MouseButton ViewSceneVtkDialog::getRenderButton(QMouseEvent* event)
{
#ifdef WITH_VTK
  auto btn = MouseButton::NONE;
  if      (event->buttons() & Qt::LeftButton)  btn = MouseButton::LEFT;
  else if (event->buttons() & Qt::RightButton) btn = MouseButton::RIGHT;
  else if (event->buttons() & Qt::MiddleButton)   btn = MouseButton::MIDDLE;
  return btn;
#endif
  return MouseButton::NONE;
}

void ViewSceneVtkDialog::mousePressEvent(QMouseEvent* event)
{
#ifdef WITH_VTK

  const float x = static_cast<float>(event->x() - viewer_->pos().x());

  const float y = static_cast<float>(event->y() - viewer_->pos().y());

  renderer_->mousePress(x, y, getRenderButton(event));

#endif
}

void ViewSceneVtkDialog::mouseMoveEvent(QMouseEvent* event)
{
#ifdef WITH_VTK

  const float x = static_cast<float>(event->x() - viewer_->pos().x());

  const float y = static_cast<float>(event->y() - viewer_->pos().y());

  renderer_->mouseMove(x, y, getRenderButton(event));

#endif
}

void ViewSceneVtkDialog::mouseReleaseEvent(QMouseEvent* event)
{
#ifdef WITH_VTK
  renderer_->mouseRelease();
#endif
}

void ViewSceneVtkDialog::wheelEvent(QWheelEvent* event)
{
  #ifdef WITH_VTK
  renderer_->mouseWheel(event->angleDelta().y());
  #endif
}

void ViewSceneVtkDialog::initializeClippingPlaneDisplay()
{
  clippingPlaneManager_->setActive(0);

  const auto& activePlane = clippingPlaneManager_->active();
  clippingPlaneControls_->updatePlaneSettingsDisplay(activePlane.visible, activePlane.showFrame, activePlane.reverseNormal);
  clippingPlaneControls_->updatePlaneControlDisplay(activePlane.x, activePlane.y, activePlane.z, activePlane.d);
}

void ViewSceneVtkDialog::setClippingPlaneIndex(int index)
{
  clippingPlaneManager_->setActive(index);

  doClippingPlanes();
}

void ViewSceneVtkDialog::doClippingPlanes()
{
  const auto& activePlane = clippingPlaneManager_->active();
  clippingPlaneControls_->updatePlaneSettingsDisplay(activePlane.visible, activePlane.showFrame, activePlane.reverseNormal);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::setClippingPlaneVisible(bool value)
{
  clippingPlaneManager_->setActiveVisibility(value);
  updateClippingPlaneDisplay();
}

void ViewSceneVtkDialog::setClippingPlaneFrameOn(bool value)
{
  //updateModifiedGeometries();
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

void ViewSceneVtkDialog::updateClippingPlaneDisplay()
{
  //newGeometryValue(false, true);

  //impl_->delayGC_ = true;
  //if (!impl_->delayedGCRequested_)
  //{
  //  impl_->delayedGCRequested_ = true;
  //  runDelayedGC();
  //}
}
