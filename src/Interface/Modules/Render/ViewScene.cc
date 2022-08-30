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

#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Application/Version.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <Core/Logging/Log.h>
#include <Core/Thread/Mutex.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Interface/Modules/Render/ES/RendererCollaborators.h>
#include <Interface/Modules/Render/ES/RendererInterface.h>
#include <Interface/Modules/Render/ES/comp/StaticClippingPlanes.h>
#include <Interface/Modules/Render/GLWidget.h>
#include <Interface/Modules/Render/Screenshot.h>
#include <Interface/Modules/Render/ViewScene.h>
#include <Interface/Modules/Render/ViewScenePlatformCompatibility.h>
#include <Interface/Modules/Render/ViewSceneUtility.h>
#include <Interface/Modules/Render/ViewSceneControlsDock.h>
#include <Interface/Modules/Base/CustomWidgets/CTK/ctkPopupWidget.h>
#include <es-log/trace-log.h>
#include <QOpenGLContext>
#include <gl-platform/GLPlatform.hpp>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Graphics::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms::Render;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Render;
using namespace SCIRun::Render::Gui;
using namespace SCIRun::Modules::Render;

namespace SCIRun {
namespace Gui {
  enum class WidgetColor
  {
    RED,
    GREEN,
    BLUE
  };

  class ScopedWidgetColorChanger
  {
  public:
  ScopedWidgetColorChanger(WidgetHandle widget, WidgetColor color)
    : widget_(widget)
    {
      backupColorValues();
      if (widget_)
      {
        backupColorValues();
        switch (color)
        {
        case WidgetColor::RED:
          colorWidgetRed();
          break;
        case WidgetColor::GREEN:
          colorWidgetGreen();
          break;
        case WidgetColor::BLUE:
          colorWidgetBlue();
          break;
        }
      }
    }

    ~ScopedWidgetColorChanger()
    {
      if (widget_)
        colorWidget(previousAmbientColor_, previousDiffuseColor_, previousSpecularColor_);
    }

  private:
    WidgetHandle widget_;
    glm::vec4                                         previousDiffuseColor_  {0.0};
    glm::vec4                                         previousSpecularColor_ {0.0};
    glm::vec4                                         previousAmbientColor_  {0.0};

    void colorWidget(glm::vec4 ambient, glm::vec4 diffuse, glm::vec4 specular)
    {
      for (auto& pass : widget_->passes())
      {
        pass.addUniform("uAmbientColor", ambient);
        pass.addUniform("uDiffuseColor", diffuse);
        pass.addUniform("uSpecularColor", specular);
      }
    }

    void colorWidgetRed()
    {
      colorWidget(glm::vec4{0.1f, 0.0f, 0.0f, 1.0f},
                  glm::vec4{1.0f, 0.0f, 0.0f, 1.0f},
                  glm::vec4{0.1f, 0.0f, 0.0f, 1.0f});
    }

    void colorWidgetGreen()
    {
      colorWidget(glm::vec4{0.0f, 0.1f, 0.0f, 1.0f},
                  glm::vec4{0.0f, 1.0f, 0.0f, 1.0f},
                  glm::vec4{0.0f, 0.1f, 0.0f, 1.0f});
    }

    void colorWidgetBlue()
    {
      colorWidget(glm::vec4{0.0f, 0.0f, 0.1f, 1.0f},
                  glm::vec4{0.0f, 0.0f, 1.0f, 1.0f},
                  glm::vec4{0.0f, 0.0f, 0.1f, 1.0f});
    }

    void backupColorValues()
    {
      for (auto& pass : widget_->passes())
        for (auto& uniform : pass.mUniforms)
        {
          if (uniform.name == "uAmbientColor")
            previousAmbientColor_ = uniform.data;
          else if (uniform.name == "uDiffuseColor")
            previousDiffuseColor_ = uniform.data;
          else if (uniform.name == "uSpecularColor")
            previousSpecularColor_ = uniform.data;
        }
    }
  };

  class ViewSceneDialogImpl
  {
  public:
    GLWidget*                             mGLWidget                     {nullptr};  ///< GL widget containing context.
    Render::RendererWeakPtr               mSpire                        {};         ///< Instance of Spire.
    QToolBar*                             toolBar1_                      {nullptr};  ///< Tool bar.
    QToolBar*                             toolBar2_                      {nullptr};  ///< Tool bar.
    QToolBar*                             toolBar3_                      {nullptr};  ///< Tool bar.
    QComboBox*                            mDownViewBox                  {nullptr};  ///< Combo box for Down axis options.
    QComboBox*                            mUpVectorBox                  {nullptr};  ///< Combo box for Up Vector options.
    ColorOptions* colorOptions_{ nullptr };
    FogControls* fogControls_{ nullptr };
    MaterialsControls* materialsControls_{ nullptr };
    ViewAxisChooserControls* viewAxisChooser_{nullptr};
    ObjectSelectionControls* objectSelectionControls_{nullptr};
    OrientationAxesControls* orientationAxesControls_{nullptr};
    ScreenshotControls* screenshotControls_{nullptr};
    ScaleBarControls* scaleBarControls_{nullptr};
    ClippingPlaneControls* clippingPlaneControls_{nullptr};
    InputControls* inputControls_{nullptr};
    CameraLockControls* cameraLockControls_{nullptr};
    DeveloperControls* developerControls_{nullptr};
    static constexpr int NUM_LIGHTS = 4;
    std::array<LightControls*, NUM_LIGHTS> lightControls_;
    CompositeLightControls* secondaryLightControlContainer_{nullptr};
    QLabel* statusLabel_{nullptr};
    QPushButton* autoRotateButton_{nullptr};
    QPushButton* fogButton_{nullptr};

    SharedPointer<ScopedWidgetColorChanger> widgetColorChanger_         {};
    Render::PreviousWidgetSelectionInfo previousWidgetInfo_;

    bool                                  shown_                        {false};
    bool                                  delayGC_                      {false};
    bool                                  delayedGCRequested_           {false};
    bool                                  invertZoom_                   {};
    bool                                  shiftdown_                    {false};
    bool                                  mouseButtonPressed_           {false};
    Graphics::Datatypes::WidgetHandle     selectedWidget_;
    Core::Datatypes::WidgetMovement       movementType_ {Core::Datatypes::NONE};

    bool initializeClippingPlanes_{true};

    const static int                      delayAfterModuleExecution_    {200};
    const static int                      delayAfterWidgetColorRestored_ {50};
    int                                   delayAfterLastSelection_      {50};
    float                                 clippingPlaneColors_[6][3]    {{0.7f, 0.2f, 0.1f}, {0.8f, 0.5f, 0.3f},
                                                                         {0.8f, 0.8f, 0.5f}, {0.4f, 0.7f, 0.3f},
                                                                         {0.2f, 0.4f, 0.5f}, {0.5f, 0.3f, 0.5f}};

    std::optional<QPoint> savedPos_;
    QColor                                bgColor_                      {};
    ScaleBarData                              scaleBar_                     {};
    Render::ClippingPlaneManagerPtr clippingPlaneManager_;
    class Screenshot*                     screenshotTaker_              {nullptr};
    bool                                  saveScreenshotOnNewGeometry_  {false};
    bool                                  pulledSavedVisibility_        {false};
    QTimer                                resizeTimer_                  {};
    std::atomic<bool>                     pushingCameraState_           {false};
    glm::vec2 previousAutoRotate_ {0,0};

    Modules::Visualization::TextBuilder               textBuilder_        {};
    Graphics::Datatypes::GeometryHandle               scaleBarGeom_       {};
    std::vector<Graphics::Datatypes::GeometryHandle>  clippingPlaneGeoms_ {};
    std::vector<Graphics::Datatypes::WidgetHandle>    widgetHandles_      {};
    QAction*                                          lockRotation_       {nullptr};
    QAction*                                          lockPan_            {nullptr};
    QAction*                                          lockZoom_           {nullptr};
    QPushButton*                                      controlLock_        {nullptr};
    QPushButton*                                      autoViewButton_     {nullptr};
    QPushButton*                                      viewBarBtn_         {nullptr};
    QPushButton* toolBar1Position_ {nullptr};
    QPushButton* toolBar2Position_ {nullptr};
    QPushButton* toolBar3Position_ {nullptr};

    std::vector<ViewSceneDialog*>                     viewScenesToUpdate  {};

    std::unique_ptr<Core::GeometryIDGenerator> gid_;
    std::string name_;

    std::unique_ptr<VisibleItemManager> visibleItems_;
    bool isFullScreen_ {false};
    std::function<bool(bool)> fullScreenSwitcher_ = [this](bool b) { return isFullScreen_ ? !b : b; };
    ViewSceneToolBarController* toolBarController_ {nullptr};
    QMainWindow* toolbarHolder_ {nullptr};

    static const int DIMENSIONS_ = 3;
    static const int QUATERNION_SIZE_ = 4;

  };

}}

unsigned long PreviousWidgetSelectionInfo::timeSince(const std::chrono::system_clock::time_point& time) const
{
  return timeSinceEpoch(std::chrono::system_clock::now()) - timeSinceEpoch(time);
}

unsigned long PreviousWidgetSelectionInfo::timeSince(unsigned long time) const
{
  return timeSinceEpoch(std::chrono::system_clock::now()) - time;
}

unsigned long PreviousWidgetSelectionInfo::timeSinceEpoch(const std::chrono::system_clock::time_point& time) const
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

bool PreviousWidgetSelectionInfo::hasSameMousePosition(int x, int y) const
{
  return lastMousePressEventX_ == x && lastMousePressEventY_ == y;
}

bool PreviousWidgetSelectionInfo::hasSameCameraTansform(const glm::mat4& mat) const
{
  return previousCameraTransform_ == mat;
}

void PreviousWidgetSelectionInfo::widgetColorRestored()
{
  timeWidgetColorRestored_ = std::chrono::system_clock::now();
}

void PreviousWidgetSelectionInfo::selectionAttempt()
{
  timeOfLastSelectionAttempt_ = std::chrono::system_clock::now();
}

void PreviousWidgetSelectionInfo::setCameraTransform(glm::mat4 mat)
{
  previousCameraTransform_ = mat;
}

void PreviousWidgetSelectionInfo::setMousePosition(int x, int y)
{
  lastMousePressEventX_ = x;
  lastMousePressEventY_ = y;
}

void PreviousWidgetSelectionInfo::setFrameIsFinished(bool finished)
{
  frameIsFinished_ = finished;
}

bool PreviousWidgetSelectionInfo::getFrameIsFinished() const
{
  return frameIsFinished_;
}

unsigned long PreviousWidgetSelectionInfo::timeSinceWidgetColorRestored() const
{
  return timeSince(timeWidgetColorRestored_);
}

unsigned long PreviousWidgetSelectionInfo::timeSinceLastSelectionAttempt() const
{
  return timeSince(timeOfLastSelectionAttempt_);
}

void PreviousWidgetSelectionInfo::setPreviousWidget(const WidgetHandle widget)
{
  previousSelectedWidget_ = widget;
}

WidgetHandle PreviousWidgetSelectionInfo::getPreviousWidget() const
{
  return previousSelectedWidget_;
}

bool PreviousWidgetSelectionInfo::hasSameWidget(const WidgetHandle widget) const
{
  return previousSelectedWidget_ == widget;
}

bool PreviousWidgetSelectionInfo::hasPreviousWidget() const
{
  if (previousSelectedWidget_)
    return true;
  else
    return false;
}

void PreviousWidgetSelectionInfo::deletePreviousWidget()
{
  previousSelectedWidget_.reset();
}

int PreviousWidgetSelectionInfo::getPreviousMouseX() const
{
  return lastMousePressEventX_;
}

int PreviousWidgetSelectionInfo::getPreviousMouseY() const
{
  return lastMousePressEventY_;
}

namespace
{
  class DialogIdGenerator : public GeometryIDGenerator
  {
  public:
    explicit DialogIdGenerator(const std::string& name) : moduleName_(name) {}
    std::string generateGeometryID(const std::string& tag) const override
    {
      return moduleName_ + "::" + tag;
    }
  private:
    std::string moduleName_;
  };

  Transform toSciTransform(const glm::mat4& mat)
  {
    //needs transposing
    Transform t;
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        t.set_mat_val(i, j, mat[j][i]);
    return t;
  }
}

ViewSceneManager ViewSceneDialog::viewSceneManager;

ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state, QWidget* parent) :
  ModuleDialogGeneric(state, parent),
  impl_(new ViewSceneDialogImpl)
{
  impl_->clippingPlaneManager_.reset(new ClippingPlaneManager(state));
  impl_->gid_.reset(new DialogIdGenerator(name));
  impl_->name_ = name;

  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  setFocusPolicy(Qt::StrongFocus);

  setupScaleBar();

  impl_->mGLWidget = new GLWidget(parentWidget());
  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(2, 1);
  impl_->mGLWidget->setFormat(format);

  connect(impl_->mGLWidget, &GLWidget::fatalError, this, &ViewSceneDialog::fatalError);
  connect(impl_->mGLWidget, &GLWidget::finishedFrame, this, &ViewSceneDialog::frameFinished);
  connect(this, &ViewSceneDialog::mousePressSignalForGeometryObjectFeedback,
          this, &ViewSceneDialog::sendGeometryFeedbackToState);

  impl_->mSpire = RendererWeakPtr(impl_->mGLWidget->getSpire());

  //Set background Color
  const auto colorStr = state_->getValue(Parameters::BackgroundColor).toString();
  impl_->bgColor_ = checkColorSetting(colorStr, Qt::black);

  {
    auto spire = impl_->mSpire.lock();
    if (!spire)
      return;

    if (Preferences::Instance().useNewViewSceneMouseControls)
    {
      spire->setMouseMode(MouseMode::MOUSE_NEWSCIRUN);
      spire->setZoomInverted(Preferences::Instance().invertMouseZoom);
    }
    else
    {
      spire->setMouseMode(MouseMode::MOUSE_OLDSCIRUN);
    }

    spire->setBackgroundColor(impl_->bgColor_);
    spire->setClippingPlaneManager(impl_->clippingPlaneManager_);
  }

  state->connectSpecificStateChanged(Parameters::GeomData,[this](){Q_EMIT newGeometryValueForwarder();});
  connect(this, &ViewSceneDialog::newGeometryValueForwarder, this, &ViewSceneDialog::updateModifiedGeometriesAndSendScreenShot);

  state->connectSpecificStateChanged(Parameters::CameraRotation,[this](){Q_EMIT cameraRotationChangeForwarder();});
  connect(this, &ViewSceneDialog::cameraRotationChangeForwarder, this, &ViewSceneDialog::pullCameraRotation);

  state->connectSpecificStateChanged(Parameters::CameraLookAt,[this](){Q_EMIT cameraLookAtChangeForwarder();});
  connect(this, &ViewSceneDialog::cameraLookAtChangeForwarder, this, &ViewSceneDialog::pullCameraLookAt);

  state->connectSpecificStateChanged(Parameters::CameraDistance,[this](){Q_EMIT cameraDistanceChangeForwarder();});
  connect(this, &ViewSceneDialog::cameraDistanceChangeForwarder, this, &ViewSceneDialog::pullCameraDistance);

  lockMutex();

  const std::string filesystemRoot = Application::Instance().executablePath().string();
  std::string sep;
  sep += boost::filesystem::path::preferred_separator;
  Modules::Visualization::TextBuilder::setFSStrings(filesystemRoot, sep);

  impl_->resizeTimer_.setSingleShot(true);
  connect(&impl_->resizeTimer_, &QTimer::timeout, this, &ViewSceneDialog::resizingDone);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  {
    impl_->toolbarHolder_ = new QMainWindow;
    impl_->toolbarHolder_->setCentralWidget(impl_->mGLWidget);

    impl_->toolBar1_ = new QToolBar;
    impl_->toolBar1_->setMovable(true);
    impl_->toolBar1_->setFloatable(true);

    impl_->toolBar2_ = new QToolBar;
    impl_->toolBar2_->setMovable(true);
    impl_->toolBar2_->setFloatable(true);

    impl_->toolBar3_ = new QToolBar;
    impl_->toolBar3_->setMovable(true);
    impl_->toolBar3_->setFloatable(true);

    layout()->addWidget(impl_->toolbarHolder_);

    impl_->toolBarController_ = new ViewSceneToolBarController(this);
  }
  addToolBar();
  setupMaterials();
  setToolBarPositions();
  addLineEditManager(impl_->screenshotControls_->defaultScreenshotPath_, Parameters::ScreenshotDirectory);

  viewSceneManager.addViewScene(this);
}

ViewSceneDialog::~ViewSceneDialog()
{
  viewSceneManager.removeViewScene(this);
}

std::string ViewSceneDialog::getName() const
{
  return impl_->name_;
}

std::string ViewSceneDialog::toString(std::string prefix) const
{
  const auto spire = impl_->mSpire.lock();

  std::string output = "VIEW_SCENE:\n";
  prefix += "  ";

  output += prefix + "State:\n";
  output += "\n";

  output += spire->toString(prefix);
  output += "\n";

  return output;
}

void ViewSceneDialog::setToolBarPositions()
{
  auto toolBar1Position = static_cast<Qt::ToolBarArea>(state_->getValue(Parameters::ToolBarMainPosition).toInt());
  auto toolBar2Position = static_cast<Qt::ToolBarArea>(state_->getValue(Parameters::ToolBarRenderPosition).toInt());
  auto toolBar3Position = static_cast<Qt::ToolBarArea>(state_->getValue(Parameters::ToolBarAdvancedPosition).toInt());
  impl_->toolbarHolder_->addToolBar(toolBar1Position, impl_->toolBar1_);
  impl_->toolbarHolder_->addToolBar(toolBar2Position, impl_->toolBar2_);
  impl_->toolbarHolder_->addToolBar(toolBar3Position, impl_->toolBar3_);
  connect(impl_->toolBar1_, &QToolBar::topLevelChanged,
    [this](bool /*topLevel*/)
    {
      state_->setValue(Parameters::ToolBarMainPosition, static_cast<int>(whereIs(impl_->toolBar1_)));
    });
  connect(impl_->toolBar2_, &QToolBar::topLevelChanged,
    [this](bool /*topLevel*/)
    {
      state_->setValue(Parameters::ToolBarRenderPosition, static_cast<int>(whereIs(impl_->toolBar2_)));
    });
  connect(impl_->toolBar3_, &QToolBar::topLevelChanged,
    [this](bool /*topLevel*/)
    {
      state_->setValue(Parameters::ToolBarAdvancedPosition, static_cast<int>(whereIs(impl_->toolBar3_)));
    });

  impl_->toolBarController_->registerDirectionButton(impl_->toolBar1_, impl_->toolBar1Position_);
  impl_->toolBarController_->registerDirectionButton(impl_->toolBar2_, impl_->toolBar2Position_);
  impl_->toolBarController_->registerDirectionButton(impl_->toolBar3_, impl_->toolBar3Position_);
}

void ViewSceneDialog::addToolBar()
{
  impl_->toolBar1_->setContextMenuPolicy(Qt::CustomContextMenu);
  WidgetStyleMixin::toolbarStyle(impl_->toolBar1_);

  impl_->toolBar2_->setContextMenuPolicy(Qt::CustomContextMenu);
  impl_->toolBar2_->setOrientation(Qt::Vertical);
  WidgetStyleMixin::toolbarStyle(impl_->toolBar2_);

  impl_->toolBar3_->setContextMenuPolicy(Qt::CustomContextMenu);
  impl_->toolBar3_->setOrientation(Qt::Vertical);
  WidgetStyleMixin::toolbarStyle(impl_->toolBar3_);

  //TODO: main toolbar members
  addAutoViewButton();
  addObjectSelectionButton();
  addViewBarButton();
  addControlLockButton();
  addScreenshotButton();
  addAutoRotateButton();

  //TODO: render toolbar members
  addColorOptionsButton();
  addOrientationAxesButton();
  addClippingPlaneButton();
  addFogOptionsButton();
  addMaterialOptionsButton();

  addLightButtons();
  addScaleBarButton();

  // TODO: advanced tool bar members
  addCameraLocksButton();
  addInputControlButton();
  addDeveloperControlButton();


  {
    impl_->toolBar1Position_ = new QPushButton();
    impl_->toolBar1Position_->setToolTip("Switch toolbar 1 popup direction");
    addToolbarButton(impl_->toolBar1Position_, Qt::TopToolBarArea);
  }
  {
    impl_->toolBar2Position_ = new QPushButton();
    impl_->toolBar2Position_->setToolTip("Switch toolbar 2 popup direction");
    addToolbarButton(impl_->toolBar2Position_, Qt::LeftToolBarArea);
  }
  {
    impl_->toolBar3Position_ = new QPushButton();
    impl_->toolBar3Position_->setToolTip("Switch toolbar 3 popup direction");
    addToolbarButton(impl_->toolBar3Position_, Qt::RightToolBarArea);
  }

  impl_->statusLabel_ = new QLabel("");
  impl_->toolBar1_->addWidget(impl_->statusLabel_);
}

void ViewSceneDialog::setupPopupWidget(QPushButton* button, ViewSceneControlPopupWidget* underlyingWidget, QToolBar* toolbar)
{
  auto* popup = new ctkPopupWidget(button);
  button->setObjectName("Button: " + underlyingWidget->objectName());

  impl_->toolBarController_->setDefaultProperties(toolbar, popup);

  connect(this, &ViewSceneDialog::closeAllNonPinnedPopups, [popup, underlyingWidget]() { if (!underlyingWidget->pinToggleAction()->isChecked()) popup->close(); });
  connect(underlyingWidget->pinToggleAction(), &QAction::toggled, popup, &ctkPopupWidget::pinPopup);
  connect(underlyingWidget->closeAction(), &QAction::triggered, popup, &QWidget::close);

  impl_->toolBarController_->registerPopup(toolbar, popup);

  auto* popupLayout = new QVBoxLayout(popup);
  popupLayout->addWidget(underlyingWidget);
  popupLayout->setContentsMargins(4,4,4,4);
}

void ViewSceneDialog::addObjectSelectionButton()
{
  auto* objectSelectionButton = new QPushButton();
  objectSelectionButton->setIcon(QPixmap(":/general/Resources/ViewScene/selection.png"));
  impl_->objectSelectionControls_ = new ObjectSelectionControls(this);
  addToolbarButton(objectSelectionButton, Qt::TopToolBarArea, impl_->objectSelectionControls_);
}

void ViewSceneDialog::addAutoRotateButton()
{
  impl_->autoRotateButton_ = new QPushButton();
  impl_->autoRotateButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autorotate2.png"));
  connect(impl_->autoRotateButton_, &QPushButton::clicked, this, &ViewSceneDialog::toggleAutoRotate);
  auto arctrls = new AutoRotateControls(this);
  addToolbarButton(impl_->autoRotateButton_, Qt::TopToolBarArea, arctrls);
}

void ViewSceneDialog::addColorOptionsButton()
{
  auto* colorOptionsButton = new QPushButton();
  colorOptionsButton->setIcon(QPixmap(":/general/Resources/ViewScene/fillColor.png"));
  impl_->colorOptions_ = new ColorOptions(this);
  impl_->colorOptions_->setSampleColor(impl_->bgColor_);
  addToolbarButton(colorOptionsButton, Qt::LeftToolBarArea, impl_->colorOptions_);
}

void ViewSceneDialog::addLightButtons()
{
  for (int i = 0; i < ViewSceneDialogImpl::NUM_LIGHTS; ++i)
  {
    auto* lightButton = new QPushButton();
    impl_->lightControls_[i] = new LightControls(this, i, lightButton);
    fixSize(impl_->lightControls_[i]);

    if (0 == i)
    {
      lightButton->setIcon(QPixmap(":/general/Resources/ViewScene/headlight.png"));
      addToolbarButton(lightButton, Qt::LeftToolBarArea, impl_->lightControls_[i]);
    }
  }
  auto* secondaryLightButton = new QPushButton();
  secondaryLightButton->setIcon(QPixmap(":/general/Resources/ViewScene/light.png"));
  impl_->secondaryLightControlContainer_ = new CompositeLightControls(this, {impl_->lightControls_.begin() + 1, impl_->lightControls_.end()});
  addToolbarButton(secondaryLightButton, Qt::LeftToolBarArea, impl_->secondaryLightControlContainer_);
}

void ViewSceneDialog::addFogOptionsButton()
{
  impl_->fogButton_ = new QPushButton();
  impl_->fogButton_->setIcon(QPixmap(":/general/Resources/ViewScene/fog.png"));
  impl_->fogControls_ = new FogControls(this, impl_->fogButton_);
  addToolbarButton(impl_->fogButton_, Qt::LeftToolBarArea, impl_->fogControls_);
}

void ViewSceneDialog::addMaterialOptionsButton()
{
  auto* materialOptionsButton = new QPushButton();
  materialOptionsButton->setIcon(QPixmap(":/general/Resources/ViewScene/materials.png"));
  impl_->materialsControls_ = new MaterialsControls(this);
  addToolbarButton(materialOptionsButton, Qt::LeftToolBarArea, impl_->materialsControls_);
}

void ViewSceneDialog::addOrientationAxesButton()
{
  auto* orientationAxesButton = new QPushButton();
  orientationAxesButton->setIcon(QPixmap(":/general/Resources/ViewScene/axes.png"));
  impl_->orientationAxesControls_ = new OrientationAxesControls(this, orientationAxesButton);
  addToolbarButton(orientationAxesButton, Qt::LeftToolBarArea, impl_->orientationAxesControls_);
}

void ViewSceneDialog::addScaleBarButton()
{
  auto* scaleBarButton = new QPushButton();
  scaleBarButton->setIcon(QPixmap(":/general/Resources/ViewScene/scaleBar.png"));
  impl_->scaleBarControls_ = new ScaleBarControls(this, scaleBarButton);
  fixSize(impl_->scaleBarControls_);
  addToolbarButton(scaleBarButton, Qt::LeftToolBarArea, impl_->scaleBarControls_);

  impl_->scaleBarControls_->setScaleBarValues(impl_->scaleBar_);
}

void ViewSceneDialog::addCameraLocksButton()
{
  auto* cameraLocksButton = new QPushButton();
  cameraLocksButton->setIcon(QPixmap(":/general/Resources/ViewScene/link.png"));
  impl_->cameraLockControls_ = new CameraLockControls(this);
  fixSize(impl_->cameraLockControls_);
  addToolbarButton(cameraLocksButton, Qt::RightToolBarArea, impl_->cameraLockControls_);
}

void ViewSceneDialog::addToolbarButton(QWidget* widget, Qt::ToolBarArea which, ViewSceneControlPopupWidget* widgetToPopup)
{
  static const auto buttonSize = 30;
  static const auto iconSize = 22;
  widget->setFixedSize(buttonSize, buttonSize);
  auto toolbar = (which == Qt::TopToolBarArea ? impl_->toolBar1_ : (which == Qt::LeftToolBarArea ? impl_->toolBar2_ : impl_->toolBar3_)); //TODO refactor obviously

  if (auto* button = qobject_cast<QPushButton*>(widget))
  {
    button->setIconSize(QSize(iconSize, iconSize));
    if (widgetToPopup)
      setupPopupWidget(button, widgetToPopup, toolbar);
  }

  toolbar->addWidget(widget);
}

void ViewSceneDialog::setupMaterials()
{
  double ambient = state_->getValue(Parameters::Ambient).toDouble();
  double diffuse = state_->getValue(Parameters::Diffuse).toDouble();
  double specular = state_->getValue(Parameters::Specular).toDouble();
  double shine = state_->getValue(Parameters::Shine).toDouble();
  bool fogOn = state_->getValue(Parameters::FogOn).toBool();
  bool useBGColor = state_->getValue(Parameters::UseBGColor).toBool();
  double fogStart = state_->getValue(Parameters::FogStart).toDouble();
  double fogEnd = state_->getValue(Parameters::FogEnd).toDouble();
  auto colorStr = state_->getValue(Parameters::FogColor).toString();

  ColorRGB color(colorStr);

  impl_->fogControls_->setColor(QColor(color.redNormalized(), color.greenNormalized(), color.blueNormalized()));

  impl_->materialsControls_->setMaterialValues(ambient, diffuse, specular, shine, 0.0);
  impl_->fogControls_->setFogValues(fogOn, false, useBGColor, fogStart, fogEnd);

  setAmbientValue(ambient);
  setDiffuseValue(diffuse);
  setSpecularValue(specular);
  setShininessValue(shine);
  setFogUseBGColor(useBGColor);
  setFogStartValue(fogStart);
  setFogEndValue(fogEnd);
  setFogOn(fogOn);
}

void ViewSceneDialog::addAutoViewButton()
{
  impl_->autoViewButton_ = new QPushButton(this);
  impl_->autoViewButton_->setToolTip("Auto View");
  impl_->autoViewButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  impl_->autoViewButton_->setShortcut(Qt::Key_0);
  connect(impl_->autoViewButton_, &QPushButton::clicked, this, &ViewSceneDialog::autoViewClicked);
  addToolbarButton(impl_->autoViewButton_, Qt::TopToolBarArea);
}

void ViewSceneDialog::addScreenshotButton()
{
  auto* screenshotButton = new QPushButton(this);
  screenshotButton->setToolTip("Take Screenshot");
  screenshotButton->setIcon(QPixmap(":/general/Resources/ViewScene/screenshot.png"));
  screenshotButton->setShortcut(Qt::Key_F12);
  connect(screenshotButton, &QPushButton::clicked, this, &ViewSceneDialog::quickScreenshotClicked);
  impl_->screenshotControls_ = new ScreenshotControls(this);
  addToolbarButton(screenshotButton, Qt::TopToolBarArea, impl_->screenshotControls_);
}

using V = glm::vec3;
using P = std::tuple<V, V>;
using InnerMap = std::map<QString, P>;
static const std::map<QString, InnerMap> axisViewParams = {
  {"+X", InnerMap {
    { "+Y", P(V( 1, 0, 0), V( 0, 1, 0)) },
    { "-Y", P(V( 1, 0, 0), V( 0,-1, 0)) },
    { "+Z", P(V( 1, 0, 0), V( 0, 0, 1)) },
    { "-Z", P(V( 1, 0, 0), V( 0, 0,-1)) }
  }},
  {"-X", InnerMap {
    { "+Y", P(V(-1, 0, 0), V( 0, 1, 0)) },
    { "-Y", P(V(-1, 0, 0), V( 0,-1, 0)) },
    { "+Z", P(V(-1, 0, 0), V( 0, 0, 1)) },
    { "-Z", P(V(-1, 0, 0), V( 0, 0,-1)) }
  }},
  {"+Y", InnerMap {
    { "+X", P(V( 0, 1, 0), V( 1, 0, 0)) },
    { "-X", P(V( 0, 1, 0), V(-1, 0, 0)) },
    { "+Z", P(V( 0, 1, 0), V( 0, 0, 1)) },
    { "-Z", P(V( 0, 1, 0), V( 0, 0,-1)) }
  }},
  {"-Y", InnerMap {
    { "+X", P(V( 0,-1, 0), V( 1, 0, 0)) },
    { "-X", P(V( 0,-1, 0), V(-1, 0, 0)) },
    { "+Z", P(V( 0,-1, 0), V( 0, 0, 1)) },
    { "-Z", P(V( 0,-1, 0), V( 0, 0,-1)) }
  }},
  {"+Z", InnerMap {
    { "+Y", P(V(0, 0, 1), V( 0, 1, 0)) },
    { "-Y", P(V(0, 0, 1), V( 0,-1, 0)) },
    { "+X", P(V(0, 0, 1), V( 1, 0, 0)) },
    { "-X", P(V(0, 0, 1), V(-1, 0, 0)) }
  }},
  {"-Z", InnerMap {
    { "+Y", P(V(0, 0,-1), V( 0, 1, 0)) },
    { "-Y", P(V(0, 0,-1), V( 0,-1, 0)) },
    { "+X", P(V(0, 0,-1), V( 1, 0, 0)) },
    { "-X", P(V(0, 0,-1), V(-1, 0, 0)) }
  }}
};

void ViewSceneDialog::addViewBarButton()
{
  impl_->viewBarBtn_ = new QPushButton();
  impl_->viewBarBtn_->setToolTip("Show View Options");
  impl_->viewBarBtn_->setIcon(QPixmap(":/general/Resources/ViewScene/views.png"));

  impl_->viewAxisChooser_ = new ViewAxisChooserControls(this);
  addToolbarButton(impl_->viewBarBtn_, Qt::TopToolBarArea, impl_->viewAxisChooser_);
  connect(impl_->viewBarBtn_, &QPushButton::clicked, this, &ViewSceneDialog::snapToViewAxis);
}

void ViewSceneDialog::addControlLockButton()
{
  impl_->controlLock_ = new QPushButton();
  impl_->controlLock_->setToolTip("Lock specific view controls");
  impl_->controlLock_->setIcon(QPixmap(":/general/Resources/ViewScene/lockView.png"));
  auto menu = new QMenu;

  impl_->lockRotation_ = menu->addAction("Lock Rotation");
  impl_->lockRotation_->setCheckable(true);
  connect(impl_->lockRotation_, &QAction::triggered, this, &ViewSceneDialog::lockRotationToggled);

  impl_->lockPan_ = menu->addAction("Lock Panning");
  impl_->lockPan_->setCheckable(true);
  connect(impl_->lockPan_, &QAction::triggered, this, &ViewSceneDialog::lockPanningToggled);

  impl_->lockZoom_ = menu->addAction("Lock Zoom");
  impl_->lockZoom_->setCheckable(true);
  connect(impl_->lockZoom_, &QAction::triggered, this, &ViewSceneDialog::lockZoomToggled);

  menu->addSeparator();

  auto lockAll = menu->addAction("Lock All");
  connect(lockAll, &QAction::triggered, this, &ViewSceneDialog::lockAllTriggered);

  auto unlockAll = menu->addAction("Unlock All");
  connect(unlockAll, &QAction::triggered, this, &ViewSceneDialog::unlockAllTriggered);

  impl_->controlLock_->setMenu(menu);

  addToolbarButton(impl_->controlLock_, Qt::TopToolBarArea);
  impl_->controlLock_->setFixedWidth(45);
  toggleLockColor(false);
}

void ViewSceneDialog::addClippingPlaneButton()
{
  auto* clippingPlaneButton = new QPushButton();
  clippingPlaneButton->setIcon(QPixmap(":/general/Resources/ViewScene/clipping.png"));
  impl_->clippingPlaneControls_ = new ClippingPlaneControls(this, clippingPlaneButton);
  addToolbarButton(clippingPlaneButton, Qt::LeftToolBarArea, impl_->clippingPlaneControls_);
}

ClippingPlaneManager::ClippingPlaneManager(ModuleStateHandle state) : state_(state), clippingPlanes_(ClippingPlane::MaxCount)
{
}

void ViewSceneDialog::setupScaleBar()
{
  if (!state_->getValue(Parameters::ScaleBarUnitValue).toString().empty())
  {
    impl_->scaleBar_.visible = state_->getValue(Parameters::ShowScaleBar).toBool();
    impl_->scaleBar_.unit = state_->getValue(Parameters::ScaleBarUnitValue).toString();
    impl_->scaleBar_.length = state_->getValue(Parameters::ScaleBarLength).toDouble();
    impl_->scaleBar_.height = state_->getValue(Parameters::ScaleBarHeight).toDouble();
    impl_->scaleBar_.multiplier = state_->getValue(Parameters::ScaleBarMultiplier).toDouble();
    impl_->scaleBar_.numTicks = state_->getValue(Parameters::ScaleBarNumTicks).toInt();
    impl_->scaleBar_.lineWidth = state_->getValue(Parameters::ScaleBarLineWidth).toDouble();
    impl_->scaleBar_.fontSize = state_->getValue(Parameters::ScaleBarFontSize).toInt();
    impl_->scaleBar_.lineColor = state_->getValue(Parameters::ScaleBarLineColor).toDouble();
  }
  else
  {
    impl_->scaleBar_.visible = false;
    impl_->scaleBar_.unit = "mm";
    impl_->scaleBar_.length = 1.0;
    impl_->scaleBar_.height = 1.0;
    impl_->scaleBar_.multiplier = 1.0;
    impl_->scaleBar_.numTicks = 11;
    impl_->scaleBar_.lineWidth = 1.0;
    impl_->scaleBar_.fontSize = 8;
    impl_->scaleBar_.lineColor = 1.0;
  }
}

void ViewSceneDialog::addInputControlButton()
{
  auto* inputControlButton = new QPushButton();
  inputControlButton->setIcon(QPixmap(":/general/Resources/ViewScene/mouse.png"));
  impl_->inputControls_ = new InputControls(this);
  addToolbarButton(inputControlButton, Qt::RightToolBarArea, impl_->inputControls_);
}

void ViewSceneDialog::addDeveloperControlButton()
{
  auto* devControlButton = new QPushButton();
  devControlButton->setIcon(QPixmap(":/general/Resources/ViewScene/devel.png"));
  impl_->developerControls_ = new DeveloperControls(this);
  addToolbarButton(devControlButton, Qt::RightToolBarArea, impl_->developerControls_);
}

void ViewSceneDialog::pullCameraState()
{
  pullCameraDistance();
  pullCameraLookAt();
  pullCameraRotation();
}

void ViewSceneDialog::pullCameraRotation()
{
  if (impl_->pushingCameraState_)
    return;
  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  glm::quat q;
  auto rotVariable = state_->getValue(Parameters::CameraRotation);
  if (rotVariable.value().type() == typeid(std::string)) // Legacy interpreter for networks that have this stored as string
    q = ViewSceneUtility::stringToQuat(state_->getValue(Parameters::CameraRotation).toString());
  else
  {
    auto rotation = toDoubleVector(rotVariable.toVector());
    if (rotation.size() == ViewSceneDialogImpl::QUATERNION_SIZE_)
      q = glm::normalize(glm::quat(rotation[0], rotation[1], rotation[2], rotation[3]));
    else
      THROW_INVALID_ARGUMENT("CameraRotation must have " + std::to_string(ViewSceneDialogImpl::QUATERNION_SIZE_) +
                             " values. " + std::to_string(rotation.size()) + " values were provided.");
  }

  spire->setCameraRotation(q);
}

void ViewSceneDialog::pullCameraLookAt()
{
  if (impl_->pushingCameraState_) return;
  auto spire = impl_->mSpire.lock();
  if (!spire) return;

  auto lookAtVariable = state_->getValue(Parameters::CameraLookAt);
  if (lookAtVariable.value().type() == typeid(std::string)) // Legacy interpreter for networks that have this stored as string
  {
    auto lookAtPoint = pointFromString(lookAtVariable.toString());
    spire->setCameraLookAt(glm::vec3(lookAtPoint[0], lookAtPoint[1], lookAtPoint[2]));
  }
  else
  {
    auto lookAt = toDoubleVector(lookAtVariable.toVector());
    if (lookAt.size() == ViewSceneDialogImpl::DIMENSIONS_)
      spire->setCameraLookAt(glm::vec3(lookAt[0], lookAt[1], lookAt[2]));
    else
      THROW_INVALID_ARGUMENT("CameraLookAt must have " + std::to_string(ViewSceneDialogImpl::DIMENSIONS_) + " values. "
                             + std::to_string(lookAt.size()) + " values were provided.");
  }
}

void ViewSceneDialog::pullCameraDistance()
{
  if (impl_->pushingCameraState_)
    return;
  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  double distance = state_->getValue(Parameters::CameraDistance).toDouble();
  double distanceMin = state_->getValue(Parameters::CameraDistanceMinimum).toDouble();
  distance = std::max(std::abs(distance), distanceMin);
  spire->setCameraDistance(distance);
}

void ViewSceneDialog::pushCameraState()
{
  pushCameraDistance();
  pushCameraLookAt();
  pushCameraRotation();
}

void ViewSceneDialog::pushCameraDistance()
{
  impl_->pushingCameraState_ = true;
  const auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  state_->setValue(Parameters::CameraDistance, static_cast<double>(spire->getCameraDistance()));
  impl_->pushingCameraState_ = false;
}

void ViewSceneDialog::pushCameraLookAt()
{
  impl_->pushingCameraState_ = true;
  const auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  const auto v = spire->getCameraLookAt();
  auto lookAt = makeAnonymousVariableList(static_cast<double>(v.x), static_cast<double>(v.y), static_cast<double>(v.z));
  state_->setValue(Parameters::CameraLookAt, lookAt);
  impl_->pushingCameraState_ = false;
}

void ViewSceneDialog::pushCameraRotation()
{
  impl_->pushingCameraState_ = true;
  const auto spire = impl_->mSpire.lock();
  if (!spire) return;

  auto q = spire->getCameraRotation();
  state_->setValue(Parameters::CameraRotation, makeAnonymousVariableList(q.w, q.x, q.y, q.z));
  impl_->pushingCameraState_ = false;
}

namespace
{
  float toInclination(int value)
  {
    return value / 180.0f * M_PI - M_PI / 2.0f;
  }

  float toAzimuth(int value)
  {
    return value / 180.0f * M_PI - M_PI;
  }

  static const std::vector<AlgorithmParameterName> lightColorKeys =
    { Parameters::HeadLightColor, Parameters::Light1Color, Parameters::Light2Color, Parameters::Light3Color };
  static const std::vector<AlgorithmParameterName> lightInclinationKeys =
    { Parameters::HeadLightInclination, Parameters::Light1Inclination, Parameters::Light2Inclination, Parameters::Light3Inclination };
  static const std::vector<AlgorithmParameterName> lightAzimuthKeys =
    { Parameters::HeadLightAzimuth, Parameters::Light1Azimuth, Parameters::Light2Azimuth, Parameters::Light3Azimuth };
  static const std::vector<AlgorithmParameterName> lightOnKeys =
    { Parameters::HeadLightOn, Parameters::Light1On, Parameters::Light2On, Parameters::Light3On };

}

void ViewSceneDialog::setInitialLightValues()
{
  auto spire = impl_->mSpire.lock();

  for (int i = 0; i < ViewSceneDialogImpl::NUM_LIGHTS; ++i)
  {
    auto lightStr = state_->getValue(lightColorKeys[i]).toString();
    auto light = checkColorSetting(lightStr, Qt::white);
    impl_->lightControls_[i]->setColor(light);
    auto lightAzimuth = state_->getValue(lightAzimuthKeys[i]).toInt();
    auto lightInclination = state_->getValue(lightInclinationKeys[i]).toInt();
    auto lightOn = state_->getValue(lightOnKeys[i]).toBool();
    impl_->lightControls_[i]->setAdditionalLightState(lightAzimuth, lightInclination, lightOn);

    if (spire)
    {
      spire->setLightAzimuth(i, toAzimuth(lightAzimuth));
      spire->setLightInclination(i, toInclination(lightInclination));
      spire->setLightColor(i, light.redF(), light.greenF(), light.blueF());
      spire->setLightOn(i, lightOn);
    }
  }
}

void ViewSceneDialog::vsLog(const QString& msg) const
{
  if (impl_ && impl_->statusLabel_)
    impl_->statusLabel_->setText(msg);
}

void ViewSceneDialog::pullSpecial()
{
  if (!impl_->pulledSavedVisibility_)
  {
    pullCameraState();
    const auto show = state_->getValue(Parameters::ShowViewer).toBool();
    if (show && parentWidget())
    {
      parentWidget()->show();
    }

    if (parentWidget())
    {
      const auto qs = QSize(state_->getValue(Parameters::WindowSizeX).toInt(), state_->getValue(Parameters::WindowSizeY).toInt());
      parentWidget()->resize(qs);
    }

    if (parentWidget())
    {
      auto dock = qobject_cast<QDockWidget*>(parentWidget());
      const auto isFloating = state_->getValue(Parameters::IsFloating).toBool();
      if (dock)
        dock->setFloating(isFloating);

      if (isFloating)
      {
        if (impl_->savedPos_)
        {
          parentWidget()->move(*impl_->savedPos_);
        }
        else
        {
          const auto x = state_->getValue(Parameters::WindowPositionX).toInt();
          const auto y = state_->getValue(Parameters::WindowPositionY).toInt();
          parentWidget()->move(x, y);
        }
      }
    }
    impl_->clippingPlaneManager_->loadFromState();
    initializeClippingPlaneDisplay();
    initializeAxes();
    initializeVisibleObjects();
    setInitialLightValues();
    impl_->pulledSavedVisibility_ = true;
  }
}

void ViewSceneDialog::adjustToolbar(double factor)
{
  adjustToolbarForHighResolution(impl_->toolBar1_, factor);
  adjustToolbarForHighResolution(impl_->toolBar2_, factor);
  adjustToolbarForHighResolution(impl_->toolBar3_, factor);
}

QColor ViewSceneDialog::checkColorSetting(const std::string& rgb, const QColor& defaultColor)
{
  QColor newColor;
  if (!rgb.empty())
  {
    ColorRGB color(rgb);
    newColor = QColor(color.redNormalized(), color.greenNormalized(), color.blueNormalized());
  }
  else
  {
    newColor = defaultColor;
  }
  return newColor;
}

//--------------------------------------------------------------------------------------------------
//---------------- New Geometry --------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::updateAllGeometries()
{
  // If a render parameter changes we must update all of the geometries by removing and reading them.
  // This must be forced because the IDs will not have changed
  newGeometryValue(true, false);

  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;
  spire->runGCOnNextExecution();
}

void ViewSceneDialog::updateModifiedGeometries()
{
  // if we are looking for a new geometry the ID will have changed therefore we can find the
  // geometries that have changed and only remove those
  newGeometryValue(false, false);

  auto spire = impl_->mSpire.lock();
  if (!spire) return;
  spire->runGCOnNextExecution();
}

void ViewSceneDialog::updateModifiedGeometriesAndSendScreenShot()
{
  newGeometryValue(false, false);
  if (impl_->mGLWidget->isVisible() && impl_->mGLWidget->isValid())
    impl_->mGLWidget->requestFrame();
  else
    unblockExecution();

  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;
  spire->runGCOnNextExecution();
}

void ViewSceneDialog::newGeometryValue(bool forceAllObjectsToUpdate, bool clippingPlanesUpdated)
{
  //DEBUG_LOG_LINE_INFO
  LOG_DEBUG("ViewSceneDialog::newGeometryValue {} before locking", windowTitle().toStdString());
  RENDERER_LOG_FUNCTION_SCOPE;
  auto lock = makeLoggedGuard(Modules::Render::ViewSceneLockManager::get(state_.get())->stateMutex(), "mutex1 -- newGeometryValue " + windowTitle().toStdString());

  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  if (!impl_->mGLWidget->isValid())
    return;
  spire->setContext(impl_->mGLWidget->context());

  if (forceAllObjectsToUpdate)
    spire->removeAllGeomObjects();

  std::vector<std::string> validObjects;
  std::vector<GeometryBaseHandle> allGeoms;

  {
    // Grab the geomData transient value.
    auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
    if (geomDataTransient && !geomDataTransient->empty())
    {
      auto portGeometries = transient_value_cast<Modules::Render::ViewScene::GeomListPtr>(geomDataTransient);
      if (!portGeometries)
      {
        LOG_DEBUG("Logical error: ViewSceneDialog received an empty list.");
        return;
      }
      std::copy(portGeometries->begin(), portGeometries->end(), std::back_inserter(allGeoms));
    }
  }

  if (impl_->scaleBarGeom_ && impl_->scaleBar_.visible)
    allGeoms.emplace_back(impl_->scaleBarGeom_);

  if (clippingPlanesUpdated)
  {
    const auto& activePlane = impl_->clippingPlaneManager_->active();
    impl_->clippingPlaneControls_->updatePlaneControlDisplay(
      activePlane.x,
      activePlane.y,
      activePlane.z,
      activePlane.d);

    buildGeomClippingPlanes();
  }

  for (auto& plane : impl_->clippingPlaneGeoms_)
    allGeoms.emplace_back(plane);

  const auto showFieldStates = transient_value_cast<ShowFieldStatesMap>(state_->getTransientValue(Parameters::ShowFieldStates));
  auto displayNames = impl_->objectSelectionControls_->visibleItems().synchronize(allGeoms, showFieldStates);

  int port = 0;
  for (auto it = allGeoms.begin(); it != allGeoms.end(); ++it, ++port)
  {
    auto obj = *it;
    auto name = displayNames[port];
    if (impl_->objectSelectionControls_->visibleItems().isVisible(name))
    {
      const auto realObj = std::dynamic_pointer_cast<GeometryObjectSpire>(obj);
      if (realObj && spire->hasObject(obj->uniqueID()))
        validObjects.push_back(obj->uniqueID());
    }
  }

  spire->gcInvalidObjects(validObjects);

  port = 0;
  for (auto it = allGeoms.begin(); it != allGeoms.end(); ++it, ++port)
  {
    auto obj = *it;
    auto name = displayNames[port];
    if (impl_->objectSelectionControls_->visibleItems().isVisible(name))
    {
      const auto realObj = std::dynamic_pointer_cast<GeometryObjectSpire>(obj);
      if (realObj && !spire->hasObject(obj->uniqueID()))
      {
        //DEBUG_LOG_LINE_INFO
        spire->handleGeomObject(realObj, port);
      }
    }
  }

  if (clippingPlanesUpdated || impl_->initializeClippingPlanes_)
  {
    impl_->initializeClippingPlanes_ = !spire->updateClippingPlanes();
  }

  if (impl_->saveScreenshotOnNewGeometry_)
    autoSaveScreenshot();
}

void ViewSceneDialog::lockMutex()
{
  //logCritical("locking screenShotMutex--Dialog::lockMutex");
  Modules::Render::ViewSceneLockManager::get(state_.get())->screenShotMutex().lock();
}

void ViewSceneDialog::unblockExecution()
{
  auto& mutex = Modules::Render::ViewSceneLockManager::get(state_.get())->screenShotMutex();
  //logCritical("unlocking screenShotMutex--Dialog::unblockExecution");
  mutex.unlock();
  std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
  //logCritical("locking screenShotMutex--Dialog::unblockExecution");
  mutex.lock();
}

void ViewSceneDialog::frameFinished()
{
  sendScreenshotDownstreamForTesting();
  unblockExecution();
  impl_->previousWidgetInfo_.setFrameIsFinished(true);
}

void ViewSceneDialog::sendGeometryFeedbackToState(int, int, const std::string& selName)
{
  auto spire = impl_->mSpire.lock();
  auto trans = spire->getWidgetTransform();

  ViewSceneFeedback vsf;
  vsf.transform = toSciTransform(trans);
  vsf.selectionName = selName;
  vsf.movementType = impl_->movementType_;
  state_->setTransientValue(Parameters::GeometryFeedbackInfo, vsf);
}

void ViewSceneDialog::runDelayedGC()
{
  if (impl_->delayGC_)
  {
    QTimer::singleShot(200, this, &ViewSceneDialog::runDelayedGC);
  }
  else
  {
    auto spire = impl_->mSpire.lock();
    if (!spire)
      return;
    spire->runGCOnNextExecution();
    impl_->delayedGCRequested_ = false;
  }
  impl_->delayGC_ = false;
}

void ViewSceneDialog::showEvent(QShowEvent* evt)
{
  {
    const auto qs = QSize(state_->getValue(Parameters::WindowSizeX).toInt(), state_->getValue(Parameters::WindowSizeY).toInt());
    parentWidget()->resize(qs);
  }

  if (!impl_->shown_)
  {
    autoViewClicked();
    impl_->shown_ = true;
  }

  if (impl_->pulledSavedVisibility_)
  {
    ScopedWidgetSignalBlocker ssb(this);
    state_->setValue(Parameters::ShowViewer, true);
  }

  updateModifiedGeometriesAndSendScreenShot();

  ModuleDialogGeneric::showEvent(evt);
}

void ViewSceneDialog::hideEvent(QHideEvent* evt)
{
  if (impl_->pulledSavedVisibility_)
  {
    ScopedWidgetSignalBlocker ssb(this);
    state_->setValue(Parameters::ShowViewer, false);
  }

  ModuleDialogGeneric::hideEvent(evt);
}

void ViewSceneDialog::closeEvent(QCloseEvent *evt)
{
  // NOTE: At one point this was required because the renderer was
  // multi-threaded. It is likely we will run into the same issue in the
  // future. Kept for future reference.
  //glLayout->removeWidget(impl_->mGLWidget);

  impl_->mGLWidget->close();
  ModuleDialogGeneric::closeEvent(evt);
}

void ViewSceneDialog::resizeEvent(QResizeEvent *event)
{
  impl_->resizeTimer_.start(400);

  ModuleDialogGeneric::resizeEvent(event);
}

void ViewSceneDialog::resizingDone()
{
  ViewSceneFeedback vsf;
  vsf.windowSize = std::make_tuple(size().width(), size().height());
  state_->setTransientValue(Parameters::GeometryFeedbackInfo, vsf);

  state_->setValue(Parameters::WindowSizeX, size().width());
  state_->setValue(Parameters::WindowSizeY, size().height());
}

void ViewSceneDialog::postMoveEventCallback(const QPoint& p)
{
  if (!impl_->savedPos_)
    impl_->savedPos_ = QPoint{ state_->getValue(Parameters::WindowPositionX).toInt(),
      state_->getValue(Parameters::WindowPositionY).toInt() };

  if (pulling_)
    return;

  state_->setValue(Parameters::WindowPositionX, p.x());
  state_->setValue(Parameters::WindowPositionY, p.y());
}

void ViewSceneDialog::setFloatingState(bool isFloating)
{
  state_->setValue(Parameters::IsFloating, isFloating);
}

void ViewSceneDialog::inputMouseDownHelper(float x, float y)
{
  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  spire->inputMouseDown(x, y);
}

void ViewSceneDialog::inputMouseMoveHelper(MouseButton btn, float x, float y)
{
  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  spire->inputMouseMove(btn, x, y);
}

void ViewSceneDialog::inputMouseUpHelper()
{
  auto spire = impl_->mSpire.lock();
  if (!spire) return;

  spire->inputMouseUp();
  pushCameraState();
}

void ViewSceneDialog::inputMouseWheelHelper(int32_t delta)
{
  auto spire = impl_->mSpire.lock();
  if (!spire) return;

  spire->inputMouseWheel(delta);
  if (impl_->scaleBar_.visible)
  {
    updateScaleBarLength();
    impl_->scaleBarGeom_ = buildGeometryScaleBar();
    updateModifiedGeometries();
  }
  state_->setValue(Parameters::CameraDistance, static_cast<double>(spire->getCameraDistance()));
}

void ViewSceneDialog::setViewScenesToUpdate(const std::unordered_set<ViewSceneDialog*>& scenes)
{
  impl_->viewScenesToUpdate.assign(scenes.begin(), scenes.end());
}

bool ViewSceneDialog::tryWidgetSelection(int x, int y, MouseButton button)
{
  bool widgetSelected = false;
  if (canSelectWidget())
  {
    impl_->mouseButtonPressed_ = true;
    selectObject(x, y, button);
    widgetSelected = true;
    updateCursor();
  }
  return widgetSelected;
}

MouseButton SCIRun::Gui::getSpireButton(QMouseEvent* event)
{
  auto btn = MouseButton::NONE;
  if (event->buttons() & Qt::LeftButton)
    btn = MouseButton::LEFT;
  else if (event->buttons() & Qt::RightButton)
    btn = MouseButton::RIGHT;
  else if (event->buttons() & Qt::MiddleButton)
    btn = MouseButton::MIDDLE;

  return btn;
}

namespace
{
  auto xPos(QMouseEvent* e)
  {
    #ifdef SCIRUN_QT6_ENABLED
      return e->position().x();
    #else
      return e->x();
    #endif
  }

  auto yPos(QMouseEvent* e)
  {
    #ifdef SCIRUN_QT6_ENABLED
      return e->position().y();
    #else
      return e->y();
    #endif
  }
}

void ViewSceneDialog::mouseMoveEvent(QMouseEvent* event)
{
  if (!clickedInViewer(event))
    return;

  auto spire = impl_->mSpire.lock();
  if (!spire) return;

  const int x_window = xPos(event) - impl_->mGLWidget->pos().x();
  const int y_window = yPos(event) - impl_->mGLWidget->pos().y();

  const auto btn = getSpireButton(event);

  if (impl_->selectedWidget_)
  {
    spire->widgetMouseMove(x_window, y_window);
  }
  else if (!impl_->shiftdown_)
  {
    float x_ss, y_ss;
    spire->calculateScreenSpaceCoords(x_window, y_window, x_ss, y_ss);
    for (auto* vsd : impl_->viewScenesToUpdate)
      vsd->inputMouseMoveHelper(btn, x_ss, y_ss);
  }
  else
  {
    tryWidgetSelection(impl_->previousWidgetInfo_.getPreviousMouseX(),
                       impl_->previousWidgetInfo_.getPreviousMouseY(), btn);
  }
}

bool ViewSceneDialog::needToWaitForWidgetSelection()
{
  const auto lastExec = transient_value_cast<unsigned long>(state_->getTransientValue(Parameters::TimeExecutionFinished));

  return impl_->previousWidgetInfo_.timeSince(lastExec) < impl_->delayAfterModuleExecution_
    || impl_->previousWidgetInfo_.timeSinceWidgetColorRestored() < impl_->delayAfterWidgetColorRestored_
    || impl_->previousWidgetInfo_.timeSinceLastSelectionAttempt() < impl_->delayAfterLastSelection_;
}

bool ViewSceneDialog::canSelectWidget()
{
  return impl_->shiftdown_ && impl_->previousWidgetInfo_.getFrameIsFinished()
    && !impl_->mouseButtonPressed_ && !needToWaitForWidgetSelection();
}

bool ViewSceneDialog::clickedInViewer(QMouseEvent* e) const
{
  return childAt(xPos(e), yPos(e)) == impl_->mGLWidget;
}

void ViewSceneDialog::mousePressEvent(QMouseEvent* event)
{
  Q_EMIT closeAllNonPinnedPopups();
  if (!clickedInViewer(event))
  {
    return;
  }

  const auto btn = getSpireButton(event);
  if (!tryWidgetSelection(xPos(event), yPos(event), btn))
  {
    auto spire = impl_->mSpire.lock();
    if (!spire) return;

    int x_window = xPos(event) - impl_->mGLWidget->pos().x();
    int y_window = yPos(event) - impl_->mGLWidget->pos().y();

    float x_ss, y_ss;
    spire->calculateScreenSpaceCoords(x_window, y_window, x_ss, y_ss);

    for (auto* vsd : impl_->viewScenesToUpdate)
      vsd->inputMouseDownHelper(x_ss, y_ss);
  }
  impl_->previousWidgetInfo_.setMousePosition(xPos(event), yPos(event));
}

void ViewSceneDialog::mouseReleaseEvent(QMouseEvent* event)
{
  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;
  const bool widgetMoved = spire->getWidgetTransform() != glm::mat4(1.0f);
  if (impl_->selectedWidget_)
  {
    if (widgetMoved)
    {
      Q_EMIT mousePressSignalForGeometryObjectFeedback(
               xPos(event), yPos(event), impl_->selectedWidget_->uniqueID());
      impl_->previousWidgetInfo_.setFrameIsFinished(false);
    }
    else
    {
      restoreObjColor();
      impl_->selectedWidget_->changeID();
      updateModifiedGeometries();
      impl_->previousWidgetInfo_.widgetColorRestored();
    }

    unblockExecution();
    impl_->previousWidgetInfo_.setPreviousWidget(impl_->selectedWidget_);
    impl_->selectedWidget_.reset();
    spire->widgetMouseUp();
    updateCursor();
  }
  else if (!impl_->shiftdown_)
  {
    for (auto* vsd : impl_->viewScenesToUpdate)
      vsd->inputMouseUpHelper();
  }

  impl_->mouseButtonPressed_ = false;
}

void ViewSceneDialog::wheelEvent(QWheelEvent* event)
{
  if (!impl_->selectedWidget_)
  {
    for (auto* vsd : impl_->viewScenesToUpdate)
    {
      vsd->inputMouseWheelHelper(event->angleDelta().y());
    }
  }
}

void ViewSceneDialog::keyPressEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    impl_->shiftdown_ = true;
    updateCursor();
    break;
  default: ;
  }
}

void ViewSceneDialog::keyReleaseEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    impl_->shiftdown_ = false;
    updateCursor();
    break;
  default: ;
  }
}

void ViewSceneDialog::focusOutEvent(QFocusEvent*)
{
  impl_->shiftdown_ = false;
  updateCursor();
}

void ViewSceneDialog::focusInEvent(QFocusEvent*)
{
  updateCursor();
}

void ViewSceneDialog::updateCursor()
{
  if (impl_->selectedWidget_)
    setCursor(Qt::ClosedHandCursor);
  else if (impl_->shiftdown_)
    setCursor(Qt::OpenHandCursor);
  else
    setCursor(Qt::ArrowCursor);
}

//--------------------------------------------------------------------------------------------------
//---------------- Camera --------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::snapToViewAxis()
{
  auto upName = impl_->viewAxisChooser_->upVectorComboBox_->currentText();
  if (upName.isEmpty())
    return;

  glm::vec3 up, view;
  std::tie(view, up) = axisViewParams.at(impl_->viewAxisChooser_->currentAxis()).at(upName);

  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  spire->setView(view, up);

  pushCameraState();
}

void ViewSceneDialog::autoViewClicked()
{
  auto spire = impl_->mSpire.lock();
  if (!spire) return;

  spire->doAutoView();

  pushCameraState();
}

void ViewSceneDialog::menuMouseControlChanged(int index)
{
  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;

  if (index == 0)
  {
    spire->setMouseMode(MouseMode::MOUSE_OLDSCIRUN);
    Preferences::Instance().useNewViewSceneMouseControls.setValue(false);
  }
  else
  {
    spire->setMouseMode(MouseMode::MOUSE_NEWSCIRUN);
    Preferences::Instance().useNewViewSceneMouseControls.setValue(true);
  }
  impl_->inputControls_->updateZoomOptionVisibility();
}

void ViewSceneDialog::invertZoomClicked(bool value)
{
  auto spire = impl_->mSpire.lock();
  spire->setZoomInverted(value);
  Preferences::Instance().invertMouseZoom.setValue(value);
}

void ViewSceneDialog::adjustZoomSpeed(int value)
{
  auto spire = impl_->mSpire.lock();
  spire->setZoomSpeed(value);
}

namespace
{
  QString buttonStyleSheet(bool active, const QString& activeColor = "red")
  {
    QString color = active ? activeColor : "rgb(66,66,69)";
    return "QPushButton { background-color: " + color + "; }";
  }
}

void ViewSceneDialog::toggleLockColor(bool locked)
{
  impl_->controlLock_->setStyleSheet(buttonStyleSheet(locked));
  impl_->autoViewButton_->setDisabled(locked);
}

void ViewSceneDialog::lockRotationToggled()
{
  impl_->mGLWidget->setLockRotation(impl_->lockRotation_->isChecked());
  toggleLockColor(impl_->lockRotation_->isChecked() || impl_->lockPan_->isChecked() || impl_->lockZoom_->isChecked());
}

void ViewSceneDialog::lockPanningToggled()
{
  impl_->mGLWidget->setLockPanning(impl_->lockPan_->isChecked());
  toggleLockColor(impl_->lockRotation_->isChecked() || impl_->lockPan_->isChecked() || impl_->lockZoom_->isChecked());
}

void ViewSceneDialog::lockZoomToggled()
{
  impl_->mGLWidget->setLockZoom(impl_->lockZoom_->isChecked());
  toggleLockColor(impl_->lockRotation_->isChecked() || impl_->lockPan_->isChecked() || impl_->lockZoom_->isChecked());
}

void ViewSceneDialog::lockAllTriggered()
{
  impl_->lockRotation_->setChecked(true);
  impl_->mGLWidget->setLockRotation(true);
  impl_->lockPan_->setChecked(true);
  impl_->mGLWidget->setLockPanning(true);
  impl_->lockZoom_->setChecked(true);
  impl_->mGLWidget->setLockZoom(true);
  toggleLockColor(true);
}

void ViewSceneDialog::unlockAllTriggered()
{
  impl_->lockRotation_->setChecked(false);
  impl_->mGLWidget->setLockRotation(false);
  impl_->lockPan_->setChecked(false);
  impl_->mGLWidget->setLockPanning(false);
  impl_->lockZoom_->setChecked(false);
  impl_->mGLWidget->setLockZoom(false);
  toggleLockColor(false);
}

void ViewSceneDialog::setAutoRotateSpeed(double speed)
{
  auto spire = impl_->mSpire.lock();
  spire->setAutoRotateSpeed(speed);
}

void ViewSceneDialog::toggleAutoRotate()
{
  auto spire = impl_->mSpire.lock();
  auto currentRotate = spire->autoRotateVector();
  if (currentRotate == glm::vec2{0,0})
  {
    spire->setAutoRotateVector(impl_->previousAutoRotate_);
    impl_->autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  }
  else
  {
    impl_->previousAutoRotate_ = currentRotate;
    spire->setAutoRotateVector({0,0});
    impl_->autoRotateButton_->setStyleSheet(buttonStyleSheet(false));
  }

  pushCameraState();
}

void ViewSceneDialog::autoRotateRight()
{
  auto spire = impl_->mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(1.0, 0.0));
  impl_->autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  pushCameraState();
}

void ViewSceneDialog::autoRotateLeft()
{
  auto spire = impl_->mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(-1.0, 0.0));
  impl_->autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  pushCameraState();
}

void ViewSceneDialog::autoRotateUp()
{
  auto spire = impl_->mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(0.0, 1.0));
  impl_->autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  pushCameraState();
}

void ViewSceneDialog::autoRotateDown()
{
  auto spire = impl_->mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(0.0, -1.0));
  impl_->autoRotateButton_->setStyleSheet(buttonStyleSheet(true, "green"));
  pushCameraState();
}

//--------------------------------------------------------------------------------------------------
//---------------- Widgets -------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::updateMeshComponentSelection(const QString& showFieldName, const QString& component, bool selected)
{
  auto name = showFieldName.toStdString();
  auto moduleId = name;
  auto renamed = name.find("(from ");
  if (renamed != std::string::npos)
    moduleId.assign(name.begin() + renamed + 6, name.end() - 1);
  MeshComponentSelectionFeedback sel(moduleId, component.toStdString(), selected);
  state_->setTransientValue(Parameters::MeshComponentSelection, sel);
}

static std::vector<WidgetHandle> filterGeomObjectsForWidgets(ViewScene::GeomListPtr geomData, VisibleItemManager& visibleItems)
{
  //getting geom list
  std::vector<WidgetHandle> objList;

  int port = 0;
  for (auto it = geomData->begin(); it != geomData->end(); ++it, ++port)
  {
    // Check if object is visible
    auto obj = *it; auto name = obj->uniqueID();
    auto displayName = QString::fromStdString(name).split(GeometryObject::delimiter).at(1);
    if (visibleItems.isVisible(displayName))
    {
      auto realObj = std::dynamic_pointer_cast<GeometryObjectSpire>(obj);
      if (realObj)
      {
        //filter objs
        bool isWidget = false;
        for (const auto& pass : realObj->passes())
        {
          if (pass.renderState.get(SCIRun::RenderState::ActionFlags::IS_WIDGET))
          {
            isWidget = true;
            break;
          }
        }
        if (isWidget)
          objList.push_back(std::dynamic_pointer_cast<WidgetBase>(realObj));
      }
    }
  }

  return objList;
}

ViewScene::GeomListPtr ViewSceneDialog::getGeomData()
{
  auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
  if (geomDataTransient && !geomDataTransient->empty())
  {
    auto geomData = transient_value_cast<Modules::Render::ViewScene::GeomListPtr>(geomDataTransient);
    if (!geomData)
    {
      LOG_DEBUG("Logical error: ViewSceneDialog received an empty list.");
      return nullptr;
    }
    return geomData;
  }
  return {};
}

void ViewSceneDialog::selectObject(const int x, const int y, MouseButton button)
{
  auto geomDataPresent = false;
  {
    LOG_DEBUG("ViewSceneDialog::asyncExecute before locking");
    auto lock = makeLoggedGuard(Modules::Render::ViewSceneLockManager::get(state_.get())->stateMutex(), "mutex1 -- selectObject");
    LOG_DEBUG("ViewSceneDialog::asyncExecute after locking");

    auto spire = impl_->mSpire.lock();
    if (!spire) return;

    const auto geomData = getGeomData();
    if (geomData)
    {
      geomDataPresent = true;
      // Search for new widgets if geometry has changed
      const bool newGeometry = state_->getValue(Parameters::HasNewGeometry).toBool();
      if (newGeometry)
      {
        impl_->widgetHandles_ = filterGeomObjectsForWidgets(geomData, impl_->objectSelectionControls_->visibleItems());
        state_->setValue(Parameters::HasNewGeometry, false);
      }

      // Search for new widget unless mouse and camera wasn't moved
      const auto adjustedX = x - impl_->mGLWidget->pos().x();
      const auto adjustedY = y - impl_->mGLWidget->pos().y();
      const auto currentCameraTransform = spire->getWorldToProjection();
      //TODO: extract function
      const bool reuseWidget = !newGeometry && impl_->previousWidgetInfo_.hasSameMousePosition(x, y)
        && impl_->previousWidgetInfo_.hasSameCameraTansform(currentCameraTransform);
      if (reuseWidget)
      {
        if (impl_->previousWidgetInfo_.hasPreviousWidget())
        {
          impl_->selectedWidget_ = impl_->previousWidgetInfo_.getPreviousWidget();
          spire->doInitialWidgetUpdate(impl_->selectedWidget_, adjustedX, adjustedY);
        }
        impl_->delayAfterLastSelection_ = 50;
      }
      else
      {
        spire->removeAllGeomObjects();
        spire->setWidgetInteractionMode(button);
        impl_->selectedWidget_ = spire->select(adjustedX, adjustedY, impl_->widgetHandles_);
        impl_->previousWidgetInfo_.setCameraTransform(currentCameraTransform);
        impl_->delayAfterLastSelection_ = 200;
      }

      if (impl_->selectedWidget_)
      {
        impl_->widgetColorChanger_ = makeShared<ScopedWidgetColorChanger>(impl_->selectedWidget_, WidgetColor::RED);
        impl_->movementType_ = impl_->selectedWidget_->movementType(yetAnotherEnumConversion(button)).base;
        impl_->selectedWidget_->changeID();
      }
      impl_->previousWidgetInfo_.deletePreviousWidget();
    }
    impl_->previousWidgetInfo_.selectionAttempt();
  }
  if (geomDataPresent)
    updateModifiedGeometries();
}

bool ViewSceneDialog::checkForSelectedWidget(WidgetHandle widget)
{
  const auto geomData = getGeomData();
  if (geomData)
  {
    const auto id = widget->uniqueID();
    for (const auto& obj : *geomData)
    {
      if (obj->uniqueID() == id)
        return true;
    }
  }
  return false;
}

void ViewSceneDialog::restoreObjColor()
{
  LOG_DEBUG("ViewSceneDialog::restoreObjColor before locking");

  auto lock = makeLoggedGuard(Modules::Render::ViewSceneLockManager::get(state_.get())->stateMutex(), "mutex1 -- restoreObjColor");
  impl_->widgetColorChanger_.reset();

  LOG_DEBUG("ViewSceneDialog::restoreObjColor after locking");
}

//--------------------------------------------------------------------------------------------------
//---------------- Clipping Planes -----------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ClippingPlaneManager::loadFromState()
{
  auto xs = toDoubleVector(state_->getValue(Parameters::ClippingPlaneX).toVector());
  auto ys = toDoubleVector(state_->getValue(Parameters::ClippingPlaneY).toVector());
  auto zs = toDoubleVector(state_->getValue(Parameters::ClippingPlaneZ).toVector());
  auto ds = toDoubleVector(state_->getValue(Parameters::ClippingPlaneD).toVector());
  auto visible = toBoolVector(state_->getValue(Parameters::ClippingPlaneEnabled).toVector());
  //auto showFrame = toBoolVector(state_->getValue(Parameters::ClippingPlaneFrameOn).toVector());
  auto reverseNormal = toBoolVector(state_->getValue(Parameters::ClippingPlaneNormalReversed).toVector());

  auto p = clippingPlanes_.begin();
  for (auto&& vals : zip(visible, reverseNormal, xs, ys, zs, ds))
  {
    boost::tie(p->visible, p->reverseNormal, p->x, p->y, p->z, p->d) = vals;
    ++p;
  }
}

void ClippingPlaneManager::setActive(int index)
{
  if (index < 0 || index >= clippingPlanes_.size())
    THROW_INVALID_ARGUMENT("Clipping plane index out of range.");
  activeIndex_ = index;
}

void ClippingPlaneManager::setActiveX(int index)
{
  clippingPlanes_[activeIndex_].x = index / 100.0;
  state_->setValue(Parameters::ClippingPlaneX, sliceWith([](const ClippingPlane& p) { return p.x; }));
}

VariableList ClippingPlaneManager::sliceWith(std::function<Variable::Value(const ClippingPlane&)> func)
{
  VariableList vl;
  std::transform(clippingPlanes_.begin(), clippingPlanes_.end(), std::back_inserter(vl), [func](const ClippingPlane& c) { return makeVariable("", func(c)); });
  return vl;
}

void ClippingPlaneManager::setActiveY(int index)
{
  clippingPlanes_[activeIndex_].y = index / 100.0;
  state_->setValue(Parameters::ClippingPlaneY, sliceWith([](const ClippingPlane& p) { return p.y; }));
}

void ClippingPlaneManager::setActiveZ(int index)
{
  clippingPlanes_[activeIndex_].z = index / 100.0;
  state_->setValue(Parameters::ClippingPlaneZ, sliceWith([](const ClippingPlane& p) { return p.z; }));
}

void ClippingPlaneManager::setActiveD(int index)
{
  clippingPlanes_[activeIndex_].d = index / 100.0;
  state_->setValue(Parameters::ClippingPlaneD, sliceWith([](const ClippingPlane& p) { return p.d; }));
}

void ClippingPlaneManager::setActiveFrameOn(bool frameOn)
{
  clippingPlanes_[activeIndex_].showFrame = frameOn;
  //TODO: state_->setValue(Parameters::ClippingPlaneFrameOn, sliceWith([](const ClippingPlane& p) { return p.showFrame; }));
}

void ClippingPlaneManager::setActiveVisibility(bool visible)
{
  clippingPlanes_[activeIndex_].visible = visible;
  state_->setValue(Parameters::ClippingPlaneEnabled, sliceWith([](const ClippingPlane& p) { return p.visible; }));
}

void ClippingPlaneManager::setActiveNormalReversed(bool normalReversed)
{
  clippingPlanes_[activeIndex_].reverseNormal = normalReversed;
  state_->setValue(Parameters::ClippingPlaneNormalReversed, sliceWith([](const ClippingPlane& p) { return p.reverseNormal; }));
}

void ViewSceneDialog::initializeClippingPlaneDisplay()
{
  impl_->clippingPlaneManager_->setActive(0);

  const auto& activePlane = impl_->clippingPlaneManager_->active();
  impl_->clippingPlaneControls_->updatePlaneSettingsDisplay(
    activePlane.visible,
    activePlane.showFrame,
    activePlane.reverseNormal);
  impl_->clippingPlaneControls_->updatePlaneControlDisplay(
    activePlane.x,
    activePlane.y,
    activePlane.z,
    activePlane.d);
}

void ViewSceneDialog::setClippingPlaneIndex(int index)
{
  impl_->clippingPlaneManager_->setActive(index);

  doClippingPlanes();
}

void ViewSceneDialog::doClippingPlanes()
{
  const auto& activePlane = impl_->clippingPlaneManager_->active();
  impl_->clippingPlaneControls_->updatePlaneSettingsDisplay(
    activePlane.visible,
    activePlane.showFrame,
    activePlane.reverseNormal);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneVisible(bool value)
{
  impl_->clippingPlaneManager_->setActiveVisibility(value);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneFrameOn(bool value)
{
  updateModifiedGeometries();
  impl_->clippingPlaneManager_->setActiveFrameOn(value);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::reverseClippingPlaneNormal(bool value)
{
  impl_->clippingPlaneManager_->setActiveNormalReversed(value);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneX(int index)
{
  impl_->clippingPlaneManager_->setActiveX(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneY(int index)
{
  impl_->clippingPlaneManager_->setActiveY(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneZ(int index)
{
  impl_->clippingPlaneManager_->setActiveZ(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneD(int index)
{
  impl_->clippingPlaneManager_->setActiveD(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::updateClippingPlaneDisplay()
{
  newGeometryValue(false, true);

  impl_->delayGC_ = true;
  if (!impl_->delayedGCRequested_)
  {
    impl_->delayedGCRequested_ = true;
    runDelayedGC();
  }
}

void ViewSceneDialog::buildGeomClippingPlanes()
{
  auto spire = impl_->mSpire.lock();
  if (!spire)
    return;
  auto* clippingPlanes = spire->getClippingPlanes();
  if (!clippingPlanes)
    return;

  impl_->clippingPlaneGeoms_.clear();
  int index = 0;
  const auto& allPlanes = impl_->clippingPlaneManager_->allPlanes();
  for (const auto& plane : clippingPlanes->clippingPlanes)
  {
    if (allPlanes[index].showFrame)
      buildGeometryClippingPlane(index, allPlanes[index].reverseNormal, plane, spire->getSceneBox());
    index++;
  }
}

void ViewSceneDialog::buildGeometryClippingPlane(int index, bool reverseNormal, const glm::vec4& plane, const BBox& bbox)
{
  if (!bbox.valid())
    return;
  Vector diag(bbox.diagonal());
  Point c(bbox.center());
  Vector n(plane.x, plane.y, plane.z);
  n.normalize();
  auto p(c + ((-plane.w) - Dot(c, n)) * n);
  if (reverseNormal)
    n = -n;
  double w, h; w = h = diag.length() / 2.0;
  Vector axis1, axis2;
  Point intersect;
  n.find_orthogonal(axis1, axis2);
  if (bbox.intersect(c, axis1, intersect))
    w = std::max(w, 2.1 * (intersect - c).length());
  if (bbox.intersect(c, axis2, intersect))
    h = std::max(h, 2.1 * (intersect - c).length());
  auto p1 = p - axis1 * w / 2.0 - axis2 * h / 2.0;
  auto p2 = p + axis1 * w / 2.0 - axis2 * h / 2.0;
  auto p3 = p + axis1 * w / 2.0 + axis2 * h / 2.0;
  auto p4 = p - axis1 * w / 2.0 + axis2 * h / 2.0;

  std::stringstream ss;
  std::string uniqueNodeID;

  Graphics::GlyphGeom glyphs;
  glyphs.addClippingPlane(p1, p2, p3, p4, 0.01 * std::min(w, h),
    10, ColorRGB(), ColorRGB());
  ss << "clipping_plane" << index <<
    p1.x() << p1.y() << p1.z() <<
    p2.x() << p2.y() << p2.z() <<
    p3.x() << p3.y() << p3.z() <<
    p4.x() << p4.y() << p4.z();
  uniqueNodeID = ss.str();
  auto colorScheme(ColorScheme::COLOR_UNIFORM);
  RenderState renState;
  renState.set(RenderState::ActionFlags::IS_ON, true);
  renState.set(RenderState::ActionFlags::USE_TRANSPARENCY, false);
  renState.defaultColor = ColorRGB(impl_->clippingPlaneColors_[index][0], impl_->clippingPlaneColors_[index][1], impl_->clippingPlaneColors_[index][2]);
  renState.set(RenderState::ActionFlags::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::ActionFlags::USE_NORMALS, true);
  renState.set(RenderState::ActionFlags::IS_WIDGET, true);
  auto geom(makeShared<GeometryObjectSpire>(*impl_->gid_, uniqueNodeID, false));
  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::ActionFlags::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, BBox(Point{}, Point{}), false, nullptr);

  Graphics::GlyphGeom glyphs2;
  glyphs2.addPlane(p1, p2, p3, p4, ColorRGB());
  ss.str("");
  ss << "clipping_plane_trans" << index <<
    p1.x() << p1.y() << p1.z() <<
    p2.x() << p2.y() << p2.z() <<
    p3.x() << p3.y() << p3.z() <<
    p4.x() << p4.y() << p4.z();
  uniqueNodeID = ss.str();
  renState.set(RenderState::ActionFlags::USE_TRANSPARENCY, true);
  renState.defaultColor = ColorRGB(1, 1, 1, 0.2);
  auto geom2(makeShared<GeometryObjectSpire>(*impl_->gid_, ss.str(), false));
  glyphs2.buildObject(*geom2, uniqueNodeID, renState.get(RenderState::ActionFlags::USE_TRANSPARENCY), 0.2,
    colorScheme, renState, BBox(Point{}, Point{}), false, nullptr);

  impl_->clippingPlaneGeoms_.push_back(geom);
  impl_->clippingPlaneGeoms_.push_back(geom2);
}



//--------------------------------------------------------------------------------------------------
//---------------- Orientation Glyph ----------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::initializeAxes()
{
  auto spire = impl_->mSpire.lock();

  {
    bool visible = state_->getValue(Parameters::AxesVisible).toBool();
    impl_->orientationAxesControls_->orientationCheckableGroupBox_->setChecked(visible);
    if (visible)
      impl_->orientationAxesControls_->toggleButton();
    spire->showOrientation(visible);
  }

  {
    int axesSize = state_->getValue(Parameters::AxesSize).toInt();
    spire->setOrientSize(axesSize);
    ScopedWidgetSignalBlocker swsb(impl_->orientationAxesControls_->orientAxisSize_);
    impl_->orientationAxesControls_->orientAxisSize_->setValue(axesSize);
  }

  {
    int axesX = state_->getValue(Parameters::AxesX).toInt();
    spire->setOrientPosX(axesX);
    ScopedWidgetSignalBlocker swsb(impl_->orientationAxesControls_->orientAxisXPos_);
    impl_->orientationAxesControls_->orientAxisXPos_->setValue(axesX);
  }

  {
    int axesY = state_->getValue(Parameters::AxesY).toInt();
    spire->setOrientPosY(axesY);
    ScopedWidgetSignalBlocker swsb(impl_->orientationAxesControls_->orientAxisYPos_);
    impl_->orientationAxesControls_->orientAxisYPos_->setValue(axesY);
  }
}

void ViewSceneDialog::showOrientationChecked(bool value)
{
  auto spire = impl_->mSpire.lock();
  spire->showOrientation(value);
  state_->setValue(Parameters::AxesVisible, value);
}

void ViewSceneDialog::setOrientAxisSize(int value)
{
  auto spire = impl_->mSpire.lock();
  spire->setOrientSize(value);
  state_->setValue(Parameters::AxesSize, value);
}

void ViewSceneDialog::setOrientAxisPosX(int pos)
{
  auto spire = impl_->mSpire.lock();
  spire->setOrientPosX(pos);
  state_->setValue(Parameters::AxesX, pos);
}

void ViewSceneDialog::setOrientAxisPosY(int pos)
{
  auto spire = impl_->mSpire.lock();
  spire->setOrientPosY(pos);
  state_->setValue(Parameters::AxesY, pos);
}

void ViewSceneDialog::setCenterOrientPos()
{
  setOrientAxisPosX(50);
  setOrientAxisPosY(50);
}

void ViewSceneDialog::setDefaultOrientPos()
{
  setOrientAxisPosX(100);
  setOrientAxisPosY(100);
}

//--------------------------------------------------------------------------------------------------
//---------------- Scale Bar -----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::setScaleBarVisible(bool value)
{
  impl_->scaleBar_.visible = value;
  state_->setValue(Parameters::ShowScaleBar, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarFontSize(int value)
{
  impl_->scaleBar_.fontSize = value;
  state_->setValue(Parameters::ScaleBarFontSize, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarUnitValue(const QString& text)
{
  impl_->scaleBar_.unit = text.toStdString();
  state_->setValue(Parameters::ScaleBarUnitValue, text.toStdString());
  setScaleBar();
}

void ViewSceneDialog::setScaleBarLength(double value)
{
  impl_->scaleBar_.length = value;
  state_->setValue(Parameters::ScaleBarLength, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarHeight(double value)
{
  impl_->scaleBar_.height = value;
  state_->setValue(Parameters::ScaleBarHeight, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarMultiplier(double value)
{
  impl_->scaleBar_.multiplier = value;
  state_->setValue(Parameters::ScaleBarMultiplier, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarNumTicks(int value)
{
  impl_->scaleBar_.numTicks = value;
  state_->setValue(Parameters::ScaleBarNumTicks, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarLineColor(double value)
{
  impl_->scaleBar_.lineColor = value;
  state_->setValue(Parameters::ScaleBarLineColor, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarLineWidth(double value)
{
  impl_->scaleBar_.lineWidth = value;
  state_->setValue(Parameters::ScaleBarLineWidth, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBar()
{
  if (impl_->scaleBar_.visible)
  {
    updateScaleBarLength();
    impl_->scaleBarGeom_ = buildGeometryScaleBar();
  }
  updateModifiedGeometries();
}

void ViewSceneDialog::updateScaleBarLength()
{
  auto spire = impl_->mSpire.lock();
  if (spire)
  {
    const auto width = spire->getScreenWidthPixels();
    const auto height = spire->getScreenHeightPixels();

    glm::vec4 p1(-impl_->scaleBar_.length / 2.0, 0.0, 0.0, 1.0);
    glm::vec4 p2(impl_->scaleBar_.length / 2.0, 0.0, 0.0, 1.0);
    glm::mat4 matIV = spire->getWorldToView();
    matIV[0][0] = 1.0; matIV[0][1] = 0.0; matIV[0][2] = 0.0;
    matIV[1][0] = 0.0; matIV[1][1] = 1.0; matIV[1][2] = 0.0;
    matIV[2][0] = 0.0; matIV[2][1] = 0.0; matIV[2][2] = 1.0;
    const auto matProj = spire->getViewToProjection();
    p1 = matProj * matIV * p1;
    p2 = matProj * matIV * p2;
    glm::vec2 p(p1.x / p1.w - p2.x / p2.w, p1.y / p1.w - p2.y / p2.w);
    glm::vec2 pp(p.x*width / 2.0,
      p.y*height / 2.0);
    impl_->scaleBar_.projLength = length(pp);
  }
}

GeometryHandle ViewSceneDialog::buildGeometryScaleBar()
{
  const int numTicks = impl_->scaleBar_.numTicks;
  double length = impl_->scaleBar_.projLength;
  const double height = impl_->scaleBar_.height;

  //figure out text length first
  const auto text_size = static_cast<size_t>(impl_->scaleBar_.fontSize);

  impl_->textBuilder_.initialize(text_size);

  //text
  std::stringstream ss;
  ss << impl_->scaleBar_.length * impl_->scaleBar_.multiplier << " " << impl_->scaleBar_.unit;
  auto oneline = ss.str();
  double text_len = 0.0;
  if (impl_->textBuilder_.isReady())
    text_len = std::get<0>(impl_->textBuilder_.getStringDims(oneline));
  text_len += 5; //add a 5-pixel gap

  std::vector<Vector> points;
  std::vector<uint32_t> indices;
  int32_t numVBOElements = 0;
  uint32_t index = 0;
  //base line
  points.emplace_back(-length - text_len, 0.0, 0.0);
  points.emplace_back(-text_len, 0.0, 0.0);
  numVBOElements += 2;
  indices.push_back(index++);
  indices.push_back(index++);
  if (numTicks > 1)
  {
    for (int i = 0; i < numTicks; ++i)
    {
      double x = -length - text_len + i*length / (numTicks - 1);
      points.emplace_back(x, 0.0, 0.0);
      points.emplace_back(x, height, 0.0);
      numVBOElements += 2;
      indices.push_back(index++);
      indices.push_back(index++);
    }
  }

  // IBO/VBOs and sizes
  uint32_t iboSize = sizeof(uint32_t) * static_cast<uint32_t>(indices.size());
  uint32_t vboSize = sizeof(float) * 3 * static_cast<uint32_t>(points.size());

  std::shared_ptr<spire::VarBuffer> iboBufferSPtr(new spire::VarBuffer(vboSize));
  std::shared_ptr<spire::VarBuffer> vboBufferSPtr(new spire::VarBuffer(iboSize));

  auto* iboBuffer = iboBufferSPtr.get();
  auto* vboBuffer = vboBufferSPtr.get();

  for (auto a : indices) iboBuffer->write(a);

  for (const auto& point : points)
  {
    vboBuffer->write(static_cast<float>(point.x()));
    vboBuffer->write(static_cast<float>(point.y()));
    vboBuffer->write(static_cast<float>(point.z()));
  }

  ss.str("");
  ss << "_scaleBar::" << impl_->scaleBar_.fontSize << impl_->scaleBar_.length << impl_->scaleBar_.height << impl_->scaleBar_.numTicks << impl_->scaleBar_.projLength << impl_->scaleBar_.lineColor;
  auto uniqueNodeID = ss.str();
  auto vboName = uniqueNodeID + "VBO";
  auto iboName = uniqueNodeID + "IBO";
  auto passName = uniqueNodeID + "Pass";

  // Construct VBO.
  const std::string shader = "Shaders/HudUniform";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.emplace_back("aPos", 3 * sizeof(float));
  std::vector<SpireSubPass::Uniform> uniforms;
  uniforms.emplace_back("uTrans", glm::vec4(1.9, 0.1, 0.0, 0.0));
  uniforms.emplace_back("uColor", glm::vec4(impl_->scaleBar_.lineColor));
  SpireVBO geomVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, BBox(Point{}, Point{}), true);

  // Construct IBO.

  SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::LINES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::ActionFlags::IS_ON, true);
  renState.set(RenderState::ActionFlags::HAS_DATA, true);
  renState.set(RenderState::ActionFlags::USE_COLORMAP, false);
  renState.set(RenderState::ActionFlags::USE_TRANSPARENCY, false);
  renState.set(RenderState::ActionFlags::IS_TEXT, true);

  SpireText text;

  SpireSubPass pass(passName, vboName, iboName, shader,
                    ColorScheme::COLOR_MAP, renState, RenderType::RENDER_VBO_IBO,
    geomVBO, geomIBO, text);

  // Add all uniforms generated above to the pass.
  for (const auto& uniform : uniforms) { pass.addUniform(uniform); }

  auto geom(makeShared<GeometryObjectSpire>(*impl_->gid_, uniqueNodeID, false));

  geom->ibos().push_back(geomIBO);
  geom->vbos().push_back(geomVBO);
  geom->passes().push_back(pass);

  //text
  if (impl_->textBuilder_.isReady())
  {
    if (impl_->textBuilder_.getFaceSize() != text_size)
      impl_->textBuilder_.setFaceSize(text_size);
    impl_->textBuilder_.setColor(1.0, 1.0, 1.0, 1.0);
    Vector shift(1.9, 0.1, 0.0);
    Vector trans(-text_len + 5, 0.0, 0.0);
    impl_->textBuilder_.printString(oneline, shift, trans, uniqueNodeID, *geom);
  }

  return geom;
}

//--------------------------------------------------------------------------------------------------
//---------------- Lights --------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::setLightColor(int index)
{
  const auto lightColor(impl_->lightControls_[index]->color());

  state_->setValue(lightColorKeys[index], ColorRGB(lightColor.redF(), lightColor.greenF(), lightColor.blueF()).toString());

  auto spire = impl_->mSpire.lock();
  if (spire)
    spire->setLightColor(index, lightColor.redF(), lightColor.greenF(), lightColor.blueF());
}

void ViewSceneDialog::setLightInclination(int index, int value)
{
  state_->setValue(lightInclinationKeys[index], value);
  auto spire = impl_->mSpire.lock();
  spire->setLightInclination(index, toInclination(value));
}

void ViewSceneDialog::toggleLight(int index, bool value)
{
  state_->setValue(lightOnKeys[index], value);
  auto spire = impl_->mSpire.lock();
  if (spire)
    spire->setLightOn(index, value);
}

void ViewSceneDialog::setLightAzimuth(int index, int value)
{
  state_->setValue(lightAzimuthKeys[index], value);
  auto spire = impl_->mSpire.lock();
  spire->setLightAzimuth(index, toAzimuth(value));
}

//--------------------------------------------------------------------------------------------------
//---------------- Materials -----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::setAmbientValue(double value)
{
  state_->setValue(Parameters::Ambient, value);
  setMaterialFactor(MatFactor::MAT_AMBIENT, value);
  updateAllGeometries();
}

void ViewSceneDialog::setDiffuseValue(double value)
{
  state_->setValue(Parameters::Diffuse, value);
  setMaterialFactor(MatFactor::MAT_DIFFUSE, value);
  updateAllGeometries();
}

void ViewSceneDialog::setSpecularValue(double value)
{
  state_->setValue(Parameters::Specular, value);
  setMaterialFactor(MatFactor::MAT_SPECULAR, value);
  updateAllGeometries();
}

void ViewSceneDialog::setShininessValue(double value)
{
  const static int maxSpecExp = 40;
  const static int minSpecExp = 1;
  state_->setValue(Parameters::Shine, value);
  //taking square of value makes the ui a little more intuitive in my opinion
  setMaterialFactor(MatFactor::MAT_SHINE, value * value * (maxSpecExp - minSpecExp) + minSpecExp);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
//---------------- Fog -----------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::setFogOn(bool value)
{
  state_->setValue(Parameters::FogOn, value);
  setFog(FogFactor::FOG_INTENSITY, value ? 1.0 : 0.0);
  updateAllGeometries();
}

void ViewSceneDialog::setFogUseBGColor(bool value)
{
  state_->setValue(Parameters::UseBGColor, value);
  if (value)
    setFogColor(glm::vec4(impl_->bgColor_.red(), impl_->bgColor_.green(), impl_->bgColor_.blue(), 1.0));
  else
  {
    auto fogColor = impl_->fogControls_->color();
    setFogColor(glm::vec4(fogColor.red(), fogColor.green(), fogColor.blue(), 1.0));
  }
  updateAllGeometries();
}

void ViewSceneDialog::assignFogColor()
{
  auto fogColor = impl_->fogControls_->color();
  state_->setValue(Parameters::FogColor, ColorRGB(fogColor.red(), fogColor.green(), fogColor.blue()).toString());
  bool useBg = state_->getValue(Parameters::UseBGColor).toBool();
  if (!useBg)
  {
    setFogColor(glm::vec4(fogColor.red(), fogColor.green(), fogColor.blue(), 1.0));
    updateAllGeometries();
  }
}

void ViewSceneDialog::setFogStartValue(double value)
{
  state_->setValue(Parameters::FogStart, value);
  setFog(FogFactor::FOG_START, value);
  updateAllGeometries();
}

void ViewSceneDialog::setFogEndValue(double value)
{
  state_->setValue(Parameters::FogEnd, value);
  setFog(FogFactor::FOG_END, value);
  updateAllGeometries();
}

void ViewSceneDialog::setMaterialFactor(MatFactor factor, double value)
{
  auto spire = impl_->mSpire.lock();
  if (spire)
    spire->setMaterialFactor(factor, value);
}

void ViewSceneDialog::setFog(FogFactor factor, double value)
{
  auto spire = impl_->mSpire.lock();
  if (spire)
    spire->setFog(factor, value);
}

void ViewSceneDialog::setFogColor(const glm::vec4 &color)
{
  auto spire = impl_->mSpire.lock();
  if (spire)
    spire->setFogColor(color/255.0f);
}

//--------------------------------------------------------------------------------------------------
//---------------- Misc. ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::assignBackgroundColor()
{
  const auto title = windowTitle() + " Choose background color";
  const auto newColor = QColorDialog::getColor(impl_->bgColor_, this, title);
  if (newColor.isValid())
  {
    impl_->bgColor_ = newColor;
    impl_->colorOptions_->setSampleColor(impl_->bgColor_);
    state_->setValue(Parameters::BackgroundColor, ColorRGB(impl_->bgColor_.red(), impl_->bgColor_.green(), impl_->bgColor_.blue()).toString());
    auto spire = impl_->mSpire.lock();
    spire->setBackgroundColor(impl_->bgColor_);
    const auto useBg = state_->getValue(Parameters::UseBGColor).toBool();
    if (useBg)
      setFogColor(glm::vec4(impl_->bgColor_.red(), impl_->bgColor_.green(), impl_->bgColor_.blue(), 1.0));
    else
    {
      const auto fogColor = impl_->fogControls_->color();
      setFogColor(glm::vec4(fogColor.red(), fogColor.green(), fogColor.blue(), 1.0));
    }
    updateAllGeometries();
  }
}

void ViewSceneDialog::setTransparencySortTypeContinuous(bool)
{
  auto spire = impl_->mSpire.lock();
  spire->setTransparencyRenderType(RenderState::TransparencySortType::CONTINUOUS_SORT);
  updateAllGeometries();
}

void ViewSceneDialog::setTransparencySortTypeUpdate(bool)
{
  auto spire = impl_->mSpire.lock();
  spire->setTransparencyRenderType(RenderState::TransparencySortType::UPDATE_SORT);
  updateAllGeometries();
}

void ViewSceneDialog::setTransparencySortTypeLists(bool)
{
  auto spire = impl_->mSpire.lock();
  spire->setTransparencyRenderType(RenderState::TransparencySortType::LISTS_SORT);
  updateAllGeometries();
}

void ViewSceneDialog::screenshotSaveAs()
{
  auto fileName = QFileDialog::getSaveFileName(impl_->mGLWidget, "Save screenshot...", QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()), "*.png");

  saveScreenshot(fileName, true);
}

void ViewSceneDialog::quickScreenshot()
{
  auto fileName = QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()) +
         QString("/%1_%2.png").arg(QString::fromStdString(getName()).replace(':', '-')).arg(QDateTime::currentDateTime().toString("yyyy.MM.dd.HHmmss.zzz"));

  saveScreenshot(fileName, true);
}

void ViewSceneDialog::setScreenshotDirectory()
{
  auto dir = QFileDialog::getExistingDirectory(this, tr("Choose Screenshot Directory"), QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()));

  state_->setValue(Parameters::ScreenshotDirectory, dir.toStdString());
}

void ViewSceneDialog::saveScreenshot(QString fileName, bool notify)
{
  if(!fileName.isEmpty())
  {
    takeScreenshot();
    if(notify)
      QMessageBox::information(nullptr, "ViewScene Screenshot", "Saving ViewScene screenshot to: " + fileName);

    impl_->screenshotTaker_->saveScreenshot(fileName);
  }
}

void ViewSceneDialog::autoSaveScreenshot()
{
  QThread::sleep(1);
  const auto file = QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()) +
                    QString("/%1_%2.png")
                    .arg(QString::fromStdString(getName()).replace(':', '-'))
                    .arg(QTime::currentTime().toString("hh.mm.ss.zzz"));

  saveScreenshot(file, false);
}

void ViewSceneDialog::sendBugReport()
{
  const QString glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  const QString gpuVersion = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

  // Temporarily save screenshot so that it can be sent over email
  QString location = QString::fromStdString(state_->getValue(Parameters::ScreenshotDirectory).toString()) + ("/scirun_bug.png");
  saveScreenshot(location, false);

  // Generate email template
  const QString askForScreenshot = "\nIMPORTANT: Make sure to attach the screenshot of the ViewScene located at "
                                   % location % "\n\n\n";
  static const QString instructions = "## For bugs, follow the template below: fill out all pertinent sections,"
    "then delete the rest of the template to reduce clutter."
    "\n### If the prerequisite is met, just delete that text as well. "
    "If they're not all met, the issue will be closed or assigned back to you.\n\n";
  static const QString prereqs = "**Prerequisite**\n* [ ] Did you [perform a cursory search](https://github.com/SCIInstitute/SCIRun/issues)"
    "to see if your bug or enhancement is already reported?\n\n";
  static const QString reportGuide = "For more information on how to write a good "
    "[bug report](https://github.com/atom/atom/blob/master/CONTRIBUTING.md#how-do-i-submit-a-good-bug-report) or"
    "[enhancement request](https://github.com/atom/atom/blob/master/CONTRIBUTING.md#how-do-i-submit-a-good-enhancement-suggestion),"
    "see the `CONTRIBUTING` guide. These links point to another project, but most of the advice holds in general.\n\n";
  static const QString describe = "**Describe the bug**\nA clear and concise description of what the bug is.\n\n";
  static const QString askForData = "**Providing sample network(s) along with input data is useful to solving your issue.**\n\n";
  static const QString reproduction = "**To Reproduce**\nSteps to reproduce the behavior:"
    "\n1. Go to '...'\n2. Click on '....'\n3. Scroll down to '....'\n4. See error\n\n";

  static const QString expectedBehavior = "**Expected behavior**\nA clear and concise description of what you expected to happen.\n\n";
  static const QString additional = "**Additional context**\nAdd any other context about the problem here.\n\n";
  const QString desktopInfo = "Desktop: " % QSysInfo::prettyProductName() % "\n";
  const QString kernelInfo = "Kernel: " % QSysInfo::kernelVersion() % "\n";
  const QString gpuInfo = "GPU: " % gpuVersion % "\n";

#ifndef OLDER_QT_SUPPORT_NEEDED // disable for older Qt 5 versions
  const QString qtInfo = "QT Version: " % QLibraryInfo::version().toString() % "\n";
  const QString glInfo = "GL Version: " % glVersion % "\n";
  const QString scirunVersionInfo = "SCIRun Version: " % QString::fromStdString(VersionInfo::GIT_VERSION_TAG) % "\n";
  const QString machineIdInfo = "Machine ID: " % QString(QSysInfo::machineUniqueId()) % "\n";

  //TODO: need generic email
  static const QString recipient = "dwhite@sci.utah.edu";
  static const QString subject = "View%20Scene%20Bug%20Report";
  QDesktopServices::openUrl(QUrl(QString("mailto:" % recipient % "?subject=" % subject % "&body=" %
                                         askForScreenshot % instructions % prereqs % reportGuide %
                                         describe % askForData % reproduction % expectedBehavior %
                                         additional % desktopInfo % kernelInfo % gpuInfo %
                                         qtInfo % glInfo % scirunVersionInfo % machineIdInfo)));
#endif
}

void ViewSceneDialog::takeScreenshot()
{
  if (!impl_->screenshotTaker_)
    impl_->screenshotTaker_ = new Screenshot(impl_->mGLWidget, this);

  impl_->screenshotTaker_->takeScreenshot();
}

void ViewSceneDialog::saveNewGeometryChanged(int state)
{
  impl_->saveScreenshotOnNewGeometry_ = state != 0;
}

void ViewSceneDialog::sendScreenshotDownstreamForTesting()
{
  takeScreenshot();
  state_->setTransientValue(Parameters::ScreenshotData, impl_->screenshotTaker_->toMatrix(), false);
}

void ViewSceneDialog::initializeVisibleObjects()
{
  impl_->objectSelectionControls_->visibleItems().initializeSavedStateMap();
}

void ViewSceneDialog::adaptToFullScreenView(bool fullScreen)
{
  impl_->isFullScreen_ = fullScreen;

  Q_EMIT fullScreenChanged();
}

bool ViewSceneDialog::isFullScreen() const
{
  return impl_->isFullScreen_;
}

Qt::ToolBarArea ViewSceneDialog::whereIs(QToolBar* toolbar) const
{
  return impl_->toolbarHolder_->toolBarArea(toolbar);
}
