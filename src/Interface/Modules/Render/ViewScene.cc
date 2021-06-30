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

  class SCISHARE ScopedWidgetColorChanger
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


}}

//--------------------------------------------------------------------------------------------------
unsigned long PreviousWidgetSelectionInfo::timeSince(const std::chrono::system_clock::time_point& time) const
{
  return timeSinceEpoch(std::chrono::system_clock::now()) - timeSinceEpoch(time);
}

//--------------------------------------------------------------------------------------------------
unsigned long PreviousWidgetSelectionInfo::timeSince(unsigned long time) const
{
  return timeSinceEpoch(std::chrono::system_clock::now()) - time;
}

//--------------------------------------------------------------------------------------------------
unsigned long PreviousWidgetSelectionInfo::timeSinceEpoch(const std::chrono::system_clock::time_point& time) const
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()).count();
}

//--------------------------------------------------------------------------------------------------
bool PreviousWidgetSelectionInfo::hasSameMousePosition(int x, int y) const
{
  return lastMousePressEventX_ == x && lastMousePressEventY_ == y;
}

//--------------------------------------------------------------------------------------------------
bool PreviousWidgetSelectionInfo::hasSameCameraTansform(const glm::mat4& mat) const
{
  return previousCameraTransform_ == mat;
}

//--------------------------------------------------------------------------------------------------
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
  clippingPlaneManager_(new ClippingPlaneManager(state)),
  gid_(new DialogIdGenerator(name)),
  name_(name)
{
  //lock
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  setFocusPolicy(Qt::StrongFocus);

  setupScaleBar();

  mGLWidget = new GLWidget(parentWidget());
  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setVersion(2, 1);
  mGLWidget->setFormat(format);

  connect(mGLWidget, SIGNAL(fatalError(const QString&)), this, SIGNAL(fatalError(const QString&)));
  connect(mGLWidget, SIGNAL(finishedFrame()), this, SLOT(frameFinished()));
  connect(this, SIGNAL(mousePressSignalForGeometryObjectFeedback(int, int, const std::string&)),
          this, SLOT(sendGeometryFeedbackToState(int, int, const std::string&)));

  mSpire = RendererWeakPtr(mGLWidget->getSpire());

  //Set background Color
  auto colorStr = state_->getValue(Parameters::BackgroundColor).toString();
  bgColor_ = checkColorSetting(colorStr, Qt::black);

  {
    auto spire = mSpire.lock();
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

    spire->setBackgroundColor(bgColor_);
    spire->setClippingPlaneManager(clippingPlaneManager_);
  }

  state->connectSpecificStateChanged(Parameters::GeomData,[this](){Q_EMIT newGeometryValueForwarder();});
  connect(this, SIGNAL(newGeometryValueForwarder()), this, SLOT(updateModifiedGeometriesAndSendScreenShot()));

  state->connectSpecificStateChanged(Parameters::CameraRotation,[this](){Q_EMIT cameraRotationChangeForwarder();});
  connect(this, SIGNAL(cameraRotationChangeForwarder()), this, SLOT(pullCameraRotation()));

  state->connectSpecificStateChanged(Parameters::CameraLookAt,[this](){Q_EMIT cameraLookAtChangeForwarder();});
  connect(this, SIGNAL(cameraLookAtChangeForwarder()), this, SLOT(pullCameraLookAt()));

  state->connectSpecificStateChanged(Parameters::CameraDistance,[this](){Q_EMIT cameraDistanceChangeForwarder();});
  connect(this, SIGNAL(cameraDistanceChangeForwarder()), this, SLOT(pullCameraDistance()));

  state->connectSpecificStateChanged(Parameters::VSMutex, [this](){Q_EMIT lockMutexForwarder();});
  connect(this, SIGNAL(lockMutexForwarder()), this, SLOT(lockMutex()));
  lockMutex();

  const std::string filesystemRoot = Application::Instance().executablePath().string();
  std::string sep;
  sep += boost::filesystem::path::preferred_separator;
  Modules::Visualization::TextBuilder::setFSStrings(filesystemRoot, sep);

  resizeTimer_.setSingleShot(true);
  connect(&resizeTimer_, SIGNAL(timeout()), this, SLOT(resizingDone()));

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  addToolBar();
  glLayout->addWidget(mGLWidget);
  glLayout->update();

  viewSceneManager.addViewScene(this);
  //viewSceneManager.moveViewSceneToGroup(this, 0);
}

ViewSceneDialog::~ViewSceneDialog()
{
  viewSceneManager.removeViewScene(this);
}

std::string ViewSceneDialog::toString(std::string prefix) const
{
  const auto spire = mSpire.lock();

  std::string output = "VIEW_SCENE:\n";
  prefix += "  ";

  output += prefix + "State:\n";
  output += "\n";

  output += spire->toString(prefix);
  output += "\n";

  return output;
}



//--------------------------------------------------------------------------------------------------
//---------------- Initialization ------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::addToolBar()
{
  mToolBar = new QToolBar(this);
  WidgetStyleMixin::toolbarStyle(mToolBar);

  addConfigurationButton();
  addConfigurationDock();
  addAutoViewButton();
  addScreenshotButton();
  addQuickScreenshotButton();

  glLayout->addWidget(mToolBar);

  addViewBar();
}

void ViewSceneDialog::addConfigurationButton()
{
  auto* configurationButton = new QPushButton();
  configurationButton->setToolTip("Open/Close Configuration Menu");
  configurationButton->setIcon(QPixmap(":/general/Resources/ViewScene/configure.png"));
  configurationButton->setShortcut(Qt::Key_F5);
  connect(configurationButton, SIGNAL(clicked(bool)), this, SLOT(configurationButtonClicked()));
  addToolbarButton(configurationButton);
}

void ViewSceneDialog::addToolbarButton(QPushButton* button)
{
  button->setFixedSize(35,35);
  button->setIconSize(QSize(25,25));
  mToolBar->addWidget(button);
}

void ViewSceneDialog::addConfigurationDock()
{
  const auto name = windowTitle() + " Configuration";
  mConfigurationDock = new ViewSceneControlsDock(name, this);
  mConfigurationDock->setHidden(true);
  mConfigurationDock->setVisible(false);

  mConfigurationDock->setSampleColor(bgColor_);
  mConfigurationDock->setScaleBarValues(scaleBar_.visible, scaleBar_.fontSize, scaleBar_.length, scaleBar_.height,
    scaleBar_.multiplier, scaleBar_.numTicks, scaleBar_.visible, QString::fromStdString(scaleBar_.unit));
  setupMaterials();
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
  fogColor_ = QColor(color.redNormalized(), color.greenNormalized(), color.blueNormalized());

  mConfigurationDock->setFogColorLabel(fogColor_);

  mConfigurationDock->setMaterialTabValues(ambient, diffuse, specular, shine,
                                           0.0, fogOn, false,
                                           useBGColor, fogStart, fogEnd);

  setAmbientValue(ambient);
  setDiffuseValue(diffuse);
  setSpecularValue(specular);
  setShininessValue(shine);
  //setEmissionValue(emission);
  //setFogOnVisibleObjects(objectsOnly);
  setFogUseBGColor(useBGColor);
  setFogStartValue(fogStart);
  setFogEndValue(fogEnd);
  setFogOn(fogOn);
}

void ViewSceneDialog::addAutoViewButton()
{
  autoViewButton_ = new QPushButton(this);
  autoViewButton_->setToolTip("Auto View");
  autoViewButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  autoViewButton_->setShortcut(Qt::Key_0);
  connect(autoViewButton_, SIGNAL(clicked(bool)), this, SLOT(autoViewClicked()));
  addToolbarButton(autoViewButton_);
}

void ViewSceneDialog::addScreenshotButton()
{
  auto* screenshotButton = new QPushButton(this);
  screenshotButton->setToolTip("Take Screenshot");
  screenshotButton->setIcon(QPixmap(":/general/Resources/ViewScene/screenshot.png"));
  screenshotButton->setShortcut(Qt::Key_F12);
  connect(screenshotButton, SIGNAL(clicked(bool)), this, SLOT(screenshotClicked()));
  addToolbarButton(screenshotButton);
}

void ViewSceneDialog::addQuickScreenshotButton()
{
  auto* quickScreenshotButton = new QPushButton(this);
  quickScreenshotButton->setToolTip("Take Quick Screenshot");
  quickScreenshotButton->setIcon(QPixmap(":/general/Resources/ViewScene/quickscreenshot.png"));
  quickScreenshotButton->setShortcut(Qt::Key_F12);
  connect(quickScreenshotButton, SIGNAL(clicked(bool)), this, SLOT(quickScreenshotClicked()));
  addToolbarButton(quickScreenshotButton);
}

void ViewSceneDialog::addViewBar()
{
  mViewBar = new QToolBar(this);

  addViewOptions();
  hideViewBar_ = true;

  mViewBar->setHidden(hideViewBar_);

  glLayout->addWidget(mViewBar);

  addViewBarButton();
  addControlLockButton();
}

using V = glm::vec3;
using P = std::tuple<V, V>;
using InnerMap = std::map<QString, P>;
static std::map<QString, InnerMap> axisViewParams;

static void initAxisViewParams()
{
  axisViewParams["+X"] = InnerMap {
    { "+Y", P(V( 1, 0, 0), V( 0, 1, 0)) },
    { "-Y", P(V( 1, 0, 0), V( 0,-1, 0)) },
    { "+Z", P(V( 1, 0, 0), V( 0, 0, 1)) },
    { "-Z", P(V( 1, 0, 0), V( 0, 0,-1)) }
  };
  axisViewParams["-X"] = InnerMap {
    { "+Y", P(V(-1, 0, 0), V( 0, 1, 0)) },
    { "-Y", P(V(-1, 0, 0), V( 0,-1, 0)) },
    { "+Z", P(V(-1, 0, 0), V( 0, 0, 1)) },
    { "-Z", P(V(-1, 0, 0), V( 0, 0,-1)) }
  };
  axisViewParams["+Y"] = InnerMap {
    { "+X", P(V( 0, 1, 0), V( 1, 0, 0)) },
    { "-X", P(V( 0, 1, 0), V(-1, 0, 0)) },
    { "+Z", P(V( 0, 1, 0), V( 0, 0, 1)) },
    { "-Z", P(V( 0, 1, 0), V( 0, 0,-1)) }
  };
  axisViewParams["-Y"] = InnerMap {
    { "+X", P(V( 0,-1, 0), V( 1, 0, 0)) },
    { "-X", P(V( 0,-1, 0), V(-1, 0, 0)) },
    { "+Z", P(V( 0,-1, 0), V( 0, 0, 1)) },
    { "-Z", P(V( 0,-1, 0), V( 0, 0,-1)) }
  };
  axisViewParams["+Z"] = InnerMap {
    { "+Y", P(V(0, 0, 1), V( 0, 1, 0)) },
    { "-Y", P(V(0, 0, 1), V( 0,-1, 0)) },
    { "+X", P(V(0, 0, 1), V( 1, 0, 0)) },
    { "-X", P(V(0, 0, 1), V(-1, 0, 0)) }
  };
  axisViewParams["-Z"] = InnerMap {
    { "+Y", P(V(0, 0,-1), V( 0, 1, 0)) },
    { "-Y", P(V(0, 0,-1), V( 0,-1, 0)) },
    { "+X", P(V(0, 0,-1), V( 1, 0, 0)) },
    { "-X", P(V(0, 0,-1), V(-1, 0, 0)) }
  };
}

void ViewSceneDialog::addViewOptions()
{
  QLabel* axisLabel = new QLabel();
  axisLabel->setText("Look Down Axis: ");
  mViewBar->addWidget(axisLabel);

  mDownViewBox = new QComboBox();
  mDownViewBox->setMinimumHeight(25);
  mDownViewBox->setMinimumWidth(60);
  mDownViewBox->setToolTip("Vector pointing out of the screen");
  mDownViewBox->addItem("+X");
  mDownViewBox->addItem("+Y");
  mDownViewBox->addItem("+Z");
  mDownViewBox->addItem("-X");
  mDownViewBox->addItem("-Y");
  mDownViewBox->addItem("-Z");
  WidgetStyleMixin::toolbarStyle(mViewBar);
  connect(mDownViewBox, SIGNAL(activated(const QString&)), this, SLOT(viewAxisSelected(const QString&)));
  mViewBar->addWidget(mDownViewBox);
  mViewBar->addSeparator();

  auto* vectorLabel = new QLabel();
  vectorLabel->setText("Up Vector: ");
  mViewBar->addWidget(vectorLabel);

  mUpVectorBox = new QComboBox();
  mUpVectorBox->setMinimumHeight(25);
  mUpVectorBox->setMinimumWidth(60);
  mUpVectorBox->setToolTip("Vector pointing up");
  connect(mUpVectorBox, SIGNAL(activated(const QString&)), this, SLOT(viewVectorSelected(const QString&)));
  mUpVectorBox->setEnabled(false);
  mViewBar->addWidget(mUpVectorBox);
  mViewBar->setMinimumHeight(35);
  initAxisViewParams();
}

void ViewSceneDialog::addViewBarButton()
{
  viewBarBtn_ = new QPushButton();
  viewBarBtn_->setToolTip("Show View Options");
  viewBarBtn_->setIcon(QPixmap(":/general/Resources/ViewScene/views.png"));
  connect(viewBarBtn_, SIGNAL(clicked(bool)), this, SLOT(viewBarButtonClicked()));
  addToolbarButton(viewBarBtn_);
}

void ViewSceneDialog::addControlLockButton()
{
  controlLock_ = new QPushButton();
  controlLock_->setToolTip("Lock specific view controls");
  controlLock_->setIcon(QPixmap(":/general/Resources/ViewScene/lockView.png"));
  auto menu = new QMenu;

  lockRotation_ = menu->addAction("Lock Rotation");
  lockRotation_->setCheckable(true);
  connect(lockRotation_, SIGNAL(triggered()), this, SLOT(lockRotationToggled()));

  lockPan_ = menu->addAction("Lock Panning");
  lockPan_->setCheckable(true);
  connect(lockPan_, SIGNAL(triggered()), this, SLOT(lockPanningToggled()));

  lockZoom_ = menu->addAction("Lock Zoom");
  lockZoom_->setCheckable(true);
  connect(lockZoom_, SIGNAL(triggered()), this, SLOT(lockZoomToggled()));

  menu->addSeparator();

  auto lockAll = menu->addAction("Lock All");
  connect(lockAll, SIGNAL(triggered()), this, SLOT(lockAllTriggered()));

  auto unlockAll = menu->addAction("Unlock All");
  connect(unlockAll, SIGNAL(triggered()), this, SLOT(unlockAllTriggered()));

  controlLock_->setMenu(menu);

  addToolbarButton(controlLock_);
  controlLock_->setFixedWidth(45);
  toggleLockColor(false);
}

ClippingPlaneManager::ClippingPlaneManager(ModuleStateHandle state) : state_(state), clippingPlanes_(ClippingPlane::MaxCount)
{

}

void ViewSceneDialog::setupScaleBar()
{
  if (!state_->getValue(Parameters::ScaleBarUnitValue).toString().empty())
  {
    scaleBar_.visible = state_->getValue(Parameters::ShowScaleBar).toBool();
    scaleBar_.unit = state_->getValue(Parameters::ScaleBarUnitValue).toString();
    scaleBar_.length = state_->getValue(Parameters::ScaleBarLength).toDouble();
    scaleBar_.height = state_->getValue(Parameters::ScaleBarHeight).toDouble();
    scaleBar_.multiplier = state_->getValue(Parameters::ScaleBarMultiplier).toDouble();
    scaleBar_.numTicks = state_->getValue(Parameters::ScaleBarNumTicks).toInt();
    scaleBar_.lineWidth = state_->getValue(Parameters::ScaleBarLineWidth).toDouble();
    scaleBar_.fontSize = state_->getValue(Parameters::ScaleBarFontSize).toInt();
  }
  else
  {
    scaleBar_.visible = false;
    scaleBar_.unit = "mm";
    scaleBar_.length = 1.0;
    scaleBar_.height = 1.0;
    scaleBar_.multiplier = 1.0;
    scaleBar_.numTicks = 11;
    scaleBar_.lineWidth = 1.0;
    scaleBar_.fontSize = 8;
  }
}

void ViewSceneDialog::pullCameraState()
{
  pullCameraDistance();
  pullCameraLookAt();
  pullCameraRotation();
}

void ViewSceneDialog::pullCameraRotation()
{
  if (pushingCameraState_)
    return;
  auto spire = mSpire.lock();
  if (!spire)
    return;

  glm::quat q;
  auto rotVariable = state_->getValue(Parameters::CameraRotation);
  if (rotVariable.value().type() == typeid(std::string)) // Legacy interpreter for networks that have this stored as string
    q = ViewSceneUtility::stringToQuat(state_->getValue(Parameters::CameraRotation).toString());
  else
  {
    auto rotation = toDoubleVector(rotVariable.toVector());
    if (rotation.size() == QUATERNION_SIZE_)
      q = glm::normalize(glm::quat(rotation[0], rotation[1], rotation[2], rotation[3]));
    else
      THROW_INVALID_ARGUMENT("CameraRotation must have " + std::to_string(QUATERNION_SIZE_) +
                             " values. " + std::to_string(rotation.size()) + " values were provided.");
  }

  spire->setCameraRotation(q);
}

void ViewSceneDialog::pullCameraLookAt()
{
  if (pushingCameraState_) return;
  auto spire = mSpire.lock();
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
    if (lookAt.size() == DIMENSIONS_)
      spire->setCameraLookAt(glm::vec3(lookAt[0], lookAt[1], lookAt[2]));
    else
      THROW_INVALID_ARGUMENT("CameraLookAt must have " + std::to_string(DIMENSIONS_) + " values. "
                             + std::to_string(lookAt.size()) + " values were provided.");
  }
}

void ViewSceneDialog::pullCameraDistance()
{
  if (pushingCameraState_) return;
  auto spire = mSpire.lock();
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
  pushingCameraState_ = true;
  const auto spire = mSpire.lock();
  if (!spire)
    return;

  state_->setValue(Parameters::CameraDistance, static_cast<double>(spire->getCameraDistance()));
  pushingCameraState_ = false;
}

void ViewSceneDialog::pushCameraLookAt()
{
  pushingCameraState_ = true;
  const auto spire = mSpire.lock();
  if (!spire)
    return;

  const auto v = spire->getCameraLookAt();
  auto lookAt = makeAnonymousVariableList(static_cast<double>(v.x), static_cast<double>(v.y), static_cast<double>(v.z));
  state_->setValue(Parameters::CameraLookAt, lookAt);
  pushingCameraState_ = false;
}

void ViewSceneDialog::pushCameraRotation()
{
  pushingCameraState_ = true;
  const auto spire = mSpire.lock();
  if (!spire) return;

  auto q = spire->getCameraRotation();
  state_->setValue(Parameters::CameraRotation, makeAnonymousVariableList(q.w, q.x, q.y, q.z));
  pushingCameraState_ = false;
}

void ViewSceneDialog::setInitialLightValues()
{
  auto light0str = state_->getValue(Parameters::HeadLightColor).toString();
  QColor light0 = checkColorSetting(light0str, Qt::white);
  int headlightAzimuth = state_->getValue(Parameters::HeadLightAzimuth).toInt();
  int headlightInclination = state_->getValue(Parameters::HeadLightInclination).toInt();

  auto light1str = state_->getValue(Parameters::Light1Color).toString();
  QColor light1 = checkColorSetting(light1str, Qt::white);
  int light1Azimuth = state_->getValue(Parameters::Light1Azimuth).toInt();
  int light1Inclination = state_->getValue(Parameters::Light1Inclination).toInt();

  auto light2str = state_->getValue(Parameters::Light2Color).toString();
  QColor light2 = checkColorSetting(light2str, Qt::white);
  int light2Azimuth = state_->getValue(Parameters::Light2Azimuth).toInt();
  int light2Inclination = state_->getValue(Parameters::Light2Inclination).toInt();

  auto light3str = state_->getValue(Parameters::Light3Color).toString();
  QColor light3 = checkColorSetting(light3str, Qt::white);
  int light3Azimuth = state_->getValue(Parameters::Light2Azimuth).toInt();
  int light3Inclination = state_->getValue(Parameters::Light2Inclination).toInt();

  auto spire = mSpire.lock();
  if (spire)
  {
    setHeadLightAzimuth(headlightAzimuth);
    setHeadLightInclination(headlightInclination);

    setLight1Azimuth(light1Azimuth);
    setLight1Inclination(light1Inclination);

    setLight2Azimuth(light2Azimuth);
    setLight2Inclination(light2Inclination);

    setLight3Azimuth(light3Azimuth);
    setLight3Inclination(light3Inclination);

    spire->setLightColor(0, light0.redF(), light0.greenF(), light0.blueF());
    spire->setLightColor(1, light1.redF(), light1.greenF(), light1.blueF());
    spire->setLightColor(2, light2.redF(), light2.greenF(), light2.blueF());
    spire->setLightColor(3, light3.redF(), light3.greenF(), light3.blueF());

    spire->setLightOn(0, state_->getValue(Parameters::HeadLightOn).toBool());
    spire->setLightOn(1, state_->getValue(Parameters::Light1On).toBool());
    spire->setLightOn(2, state_->getValue(Parameters::Light2On).toBool());
    spire->setLightOn(3, state_->getValue(Parameters::Light3On).toBool());
  }
}

void ViewSceneDialog::pullSpecial()
{
  if (!pulledSavedVisibility_)
  {
    ScopedWidgetSignalBlocker swsb(mConfigurationDock);
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
      parentWidget()->resize(qs + QSize(1,1));
    }

    if (parentWidget())
    {
      auto dock = qobject_cast<QDockWidget*>(parentWidget());
      const auto isFloating = state_->getValue(Parameters::IsFloating).toBool();
      if (dock)
        dock->setFloating(isFloating);

      if (isFloating)
      {
        if (savedPos_)
        {
          parentWidget()->move(*savedPos_);
        }
        else
        {
          const auto x = state_->getValue(Parameters::WindowPositionX).toInt();
          const auto y = state_->getValue(Parameters::WindowPositionY).toInt();
          parentWidget()->move(x, y);
        }
      }
    }
    clippingPlaneManager_->loadFromState();
    initializeClippingPlaneDisplay();
    initializeAxes();
    initializeVisibleObjects();
    setInitialLightValues();
    pulledSavedVisibility_ = true;
  }
}

void ViewSceneDialog::adjustToolbar()
{
  adjustToolbarForHighResolution(mToolBar);
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

  auto spire = mSpire.lock();
  if (!spire)
    return;
  spire->runGCOnNextExecution();
}

void ViewSceneDialog::updateModifiedGeometries()
{
  // if we are looking for a new geometry the ID will have changed therefore we can find the
  // geometries that have changed and only remove those
  newGeometryValue(false, false);

  auto spire = mSpire.lock();
  if (!spire) return;
  spire->runGCOnNextExecution();
}

void ViewSceneDialog::updateModifiedGeometriesAndSendScreenShot()
{
  newGeometryValue(false, false);
  if (mGLWidget->isVisible() && mGLWidget->isValid())
    mGLWidget->requestFrame();
  else
    unblockExecution();

  auto spire = mSpire.lock();
  if (!spire)
    return;
  spire->runGCOnNextExecution();
}

void ViewSceneDialog::newGeometryValue(bool forceAllObjectsToUpdate, bool clippingPlanesUpdated)
{
  DEBUG_LOG_LINE_INFO
  LOG_DEBUG("ViewSceneDialog::newGeometryValue {} before locking", windowTitle().toStdString());
  RENDERER_LOG_FUNCTION_SCOPE;
  Guard lock(Modules::Render::ViewScene::mutex_.get());

  auto spire = mSpire.lock();
  if (!spire)
    return;

  if (!mGLWidget->isValid())
    return;
  spire->setContext(mGLWidget->context());

  if (forceAllObjectsToUpdate)
    spire->removeAllGeomObjects();

  std::vector<std::string> validObjects;
  std::vector<GeometryBaseHandle> allGeoms;

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

  if (scaleBarGeom_ && scaleBar_.visible)
    allGeoms.push_back(scaleBarGeom_);

  if (clippingPlanesUpdated)
  {
    const auto& activePlane = clippingPlaneManager_->active();
    mConfigurationDock->updatePlaneControlDisplay(
      activePlane.x,
      activePlane.y,
      activePlane.z,
      activePlane.d);

    buildGeomClippingPlanes();
  }

  for (auto& plane : clippingPlaneGeoms_)
    allGeoms.push_back(plane);

  const auto showFieldStates = transient_value_cast<ShowFieldStatesMap>(state_->getTransientValue(Parameters::ShowFieldStates));
  auto displayNames = mConfigurationDock->visibleItems().synchronize(allGeoms, showFieldStates);

  int port = 0;
  for (auto it = allGeoms.begin(); it != allGeoms.end(); ++it, ++port)
  {
    auto obj = *it;
    auto name = displayNames[port];
    if (mConfigurationDock->visibleItems().isVisible(name))
    {
      const auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
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
    if (mConfigurationDock->visibleItems().isVisible(name))
    {
      const auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
      if (realObj && !spire->hasObject(obj->uniqueID()))
      {
        DEBUG_LOG_LINE_INFO
        spire->handleGeomObject(realObj, port);
      }
    }
  }

  if (clippingPlanesUpdated || initializeClippingPlanes_)
  {
    initializeClippingPlanes_ = !spire->updateClippingPlanes();
  }

  if (saveScreenshotOnNewGeometry_)
    screenshotClicked();
}

void ViewSceneDialog::lockMutex()
{
  auto screenShotMutex = state_->getTransientValue(Parameters::VSMutex);
  auto mutex = transient_value_cast<Mutex*>(screenShotMutex);
  if (mutex)
    mutex->lock();
}

void ViewSceneDialog::unblockExecution()
{
  auto screenShotMutex = state_->getTransientValue(Parameters::VSMutex);
  auto mutex = transient_value_cast<Mutex*>(screenShotMutex);
  if (mutex)
  {
    mutex->unlock();
    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
    mutex->lock();
  }
}

void ViewSceneDialog::frameFinished()
{
  sendScreenshotDownstreamForTesting();
  unblockExecution();
  previousWidgetInfo_.setFrameIsFinished(true);
}

void ViewSceneDialog::sendGeometryFeedbackToState(int, int, const std::string& selName)
{
  auto spire = mSpire.lock();
  auto trans = spire->getWidgetTransform();

  ViewSceneFeedback vsf;
  vsf.transform = toSciTransform(trans);
  vsf.selectionName = selName;
  vsf.movementType = movementType_;
  state_->setTransientValue(Parameters::GeometryFeedbackInfo, vsf);
}

void ViewSceneDialog::runDelayedGC()
{
  if (delayGC_)
  {
    QTimer::singleShot(200, this, &ViewSceneDialog::runDelayedGC);
  }
  else
  {
    auto spire = mSpire.lock();
    if (!spire)
      return;
    spire->runGCOnNextExecution();
    delayedGCRequested_ = false;
  }
  delayGC_ = false;
}

void ViewSceneDialog::showEvent(QShowEvent* evt)
{
  if (!shown_)
  {
    autoViewClicked();
    shown_ = true;
  }

  if (pulledSavedVisibility_)
  {
    ScopedWidgetSignalBlocker ssb(this);
    state_->setValue(Parameters::ShowViewer, true);
  }

  updateModifiedGeometriesAndSendScreenShot();

  ModuleDialogGeneric::showEvent(evt);
}

void ViewSceneDialog::hideEvent(QHideEvent* evt)
{
  mConfigurationDock->setVisible(false);

  if (pulledSavedVisibility_)
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
  //glLayout->removeWidget(mGLWidget);

  mGLWidget->close();
  ModuleDialogGeneric::closeEvent(evt);
}

void ViewSceneDialog::viewBarButtonClicked()
{
  hideViewBar_ = !hideViewBar_;
  mViewBar->setHidden(hideViewBar_);
  QString color = hideViewBar_ ? "rgb(66,66,69)" : "lightGray";
  viewBarBtn_->setStyleSheet("QPushButton { background-color: " + color + "; }");
  mDownViewBox->setCurrentIndex(0);
  mUpVectorBox->setDisabled(hideViewBar_);
  mUpVectorBox->clear();
}

void ViewSceneDialog::configurationButtonClicked()
{
  mConfigurationDock->setVisible(!mConfigurationDock->isVisible());
}

void ViewSceneDialog::resizeEvent(QResizeEvent *event)
{
  resizeTimer_.start(400);

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
  if (!savedPos_)
    savedPos_ = QPoint{ state_->getValue(Parameters::WindowPositionX).toInt(),
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
  auto spire = mSpire.lock();
  if (!spire)
    return;

  spire->inputMouseDown(x, y);
}

void ViewSceneDialog::inputMouseMoveHelper(MouseButton btn, float x, float y)
{
  auto spire = mSpire.lock();
  if (!spire)
    return;

  spire->inputMouseMove(btn, x, y);
}

void ViewSceneDialog::inputMouseUpHelper()
{
  auto spire = mSpire.lock();
  if(!spire) return;

  spire->inputMouseUp();
  pushCameraState();
}

void ViewSceneDialog::inputMouseWheelHelper(int32_t delta)
{
  auto spire = mSpire.lock();
  if(!spire) return;

  spire->inputMouseWheel(delta);
  if (scaleBar_.visible)
  {
    updateScaleBarLength();
    scaleBarGeom_ = buildGeometryScaleBar();
    updateModifiedGeometries();
  }
  state_->setValue(Parameters::CameraDistance, static_cast<double>(spire->getCameraDistance()));
}

void ViewSceneDialog::setViewScenesToUpdate(const std::unordered_set<ViewSceneDialog*>& scenes)
{
  viewScenesToUpdate.assign(scenes.begin(), scenes.end());
}

bool ViewSceneDialog::tryWidgetSelection(int x, int y, MouseButton button)
{
  bool widgetSelected = false;
  if (canSelectWidget())
  {
    mouseButtonPressed_ = true;
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

void ViewSceneDialog::mouseMoveEvent(QMouseEvent* event)
{
  if (!clickedInViewer(event))
    return;

  auto spire = mSpire.lock();
  if(!spire) return;

  const int x_window = event->x() - mGLWidget->pos().x();
  const int y_window = event->y() - mGLWidget->pos().y();

  const auto btn = getSpireButton(event);

  if (selectedWidget_)
  {
    spire->widgetMouseMove(x_window, y_window);
  }
  else if (!shiftdown_)
  {
    float x_ss, y_ss;
    spire->calculateScreenSpaceCoords(x_window, y_window, x_ss, y_ss);
    for (auto* vsd : viewScenesToUpdate)
      vsd->inputMouseMoveHelper(btn, x_ss, y_ss);
  }
  else
  {
    tryWidgetSelection(previousWidgetInfo_.getPreviousMouseX(),
                       previousWidgetInfo_.getPreviousMouseY(), btn);
  }
}

bool ViewSceneDialog::needToWaitForWidgetSelection()
{
  const auto lastExec = transient_value_cast<unsigned long>(state_->getTransientValue(Parameters::TimeExecutionFinished));

  return previousWidgetInfo_.timeSince(lastExec) < delayAfterModuleExecution_
    || previousWidgetInfo_.timeSinceWidgetColorRestored() < delayAfterWidgetColorRestored_
    || previousWidgetInfo_.timeSinceLastSelectionAttempt() < delayAfterLastSelection_;
}

bool ViewSceneDialog::canSelectWidget()
{
  return shiftdown_ && previousWidgetInfo_.getFrameIsFinished()
    && !mouseButtonPressed_ && !needToWaitForWidgetSelection();
}

bool ViewSceneDialog::clickedInViewer(QMouseEvent* e) const
{
  return childAt(e->x(), e->y()) == mGLWidget;
}

void ViewSceneDialog::mousePressEvent(QMouseEvent* event)
{
  if (!clickedInViewer(event))
    return;

  const auto btn = getSpireButton(event);
  if (!tryWidgetSelection(event->x(), event->y(), btn))
  {
    auto spire = mSpire.lock();
    if (!spire) return;

    int x_window = event->x() - mGLWidget->pos().x();
    int y_window = event->y() - mGLWidget->pos().y();

    float x_ss, y_ss;
    spire->calculateScreenSpaceCoords(x_window, y_window, x_ss, y_ss);

    for (auto* vsd : viewScenesToUpdate)
      vsd->inputMouseDownHelper(x_ss, y_ss);
  }
  previousWidgetInfo_.setMousePosition(event->x(), event->y());
}

void ViewSceneDialog::mouseReleaseEvent(QMouseEvent* event)
{
  auto spire = mSpire.lock();
  if (!spire)
    return;
  const bool widgetMoved = spire->getWidgetTransform() != glm::mat4(1.0f);
  if (selectedWidget_)
  {
    if (widgetMoved)
    {
      Q_EMIT mousePressSignalForGeometryObjectFeedback(
               event->x(), event->y(), selectedWidget_->uniqueID());
      previousWidgetInfo_.setFrameIsFinished(false);
    }
    else
    {
      restoreObjColor();
      selectedWidget_->changeID();
      updateModifiedGeometries();
      previousWidgetInfo_.widgetColorRestored();
    }

    unblockExecution();
    previousWidgetInfo_.setPreviousWidget(selectedWidget_);
    selectedWidget_.reset();
    spire->widgetMouseUp();
    updateCursor();
  }
  else if (!shiftdown_)
  {
    for (auto* vsd : viewScenesToUpdate)
      vsd->inputMouseUpHelper();
  }

  mouseButtonPressed_ = false;
}

//TODO!!!
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
void ViewSceneDialog::wheelEvent(QWheelEvent* event)
{
  if (!selectedWidget_)
  {
    for (auto* vsd : viewScenesToUpdate)
      vsd->inputMouseWheelHelper(event->delta());
  }
}

void ViewSceneDialog::keyPressEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    shiftdown_ = true;
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
    shiftdown_ = false;
    updateCursor();
    break;
  default: ;
  }
}

void ViewSceneDialog::focusOutEvent(QFocusEvent*)
{
  shiftdown_ = false;
  updateCursor();
}

void ViewSceneDialog::focusInEvent(QFocusEvent*)
{
  updateCursor();
}

void ViewSceneDialog::updateCursor()
{
  if (selectedWidget_)
    setCursor(Qt::ClosedHandCursor);
  else if (shiftdown_)
    setCursor(Qt::OpenHandCursor);
  else
    setCursor(Qt::ArrowCursor);
}


//--------------------------------------------------------------------------------------------------
//---------------- Camera --------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::viewAxisSelected(const QString& name)
{
  mUpVectorBox->clear();

  if (!name.contains("X"))
  {
    mUpVectorBox->addItem("+X");
    mUpVectorBox->addItem("-X");
  }
  if (!name.contains("Y"))
  {
    mUpVectorBox->addItem("+Y");
    mUpVectorBox->addItem("-Y");
  }
  if (!name.contains("Z"))
  {
    mUpVectorBox->addItem("+Z");
    mUpVectorBox->addItem("-Z");
  }
  mUpVectorBox->setEnabled(true);
}

void ViewSceneDialog::viewVectorSelected(const QString& name)
{
  if (name.isEmpty())
    return;

  glm::vec3 up, view;
  std::tie(view, up) = axisViewParams[mDownViewBox->currentText()][name];

  auto spire = mSpire.lock();
  if(!spire) return;

  spire->setView(view, up);

  pushCameraState();
}

void ViewSceneDialog::autoViewClicked()
{
  auto spire = mSpire.lock();
  if(!spire) return;

  spire->doAutoView();

  pushCameraState();
}

void ViewSceneDialog::menuMouseControlChanged(int index)
{
  auto spire = mSpire.lock();
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
  mConfigurationDock->updateZoomOptionVisibility();
}

void ViewSceneDialog::invertZoomClicked(bool value)
{
  auto spire = mSpire.lock();
  spire->setZoomInverted(value);
  Preferences::Instance().invertMouseZoom.setValue(value);
}

void ViewSceneDialog::adjustZoomSpeed(int value)
{
  auto spire = mSpire.lock();
  spire->setZoomSpeed(value);
}

void ViewSceneDialog::toggleLockColor(bool locked)
{
  QString color = locked ? "red" : "rgb(66,66,69)";
  controlLock_->setStyleSheet("QPushButton { background-color: " + color + "; }");
  autoViewButton_->setDisabled(locked);
}

void ViewSceneDialog::lockRotationToggled()
{
  mGLWidget->setLockRotation(lockRotation_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void ViewSceneDialog::lockPanningToggled()
{
  mGLWidget->setLockPanning(lockPan_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void ViewSceneDialog::lockZoomToggled()
{
  mGLWidget->setLockZoom(lockZoom_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void ViewSceneDialog::lockAllTriggered()
{
  lockRotation_->setChecked(true);
  mGLWidget->setLockRotation(true);
  lockPan_->setChecked(true);
  mGLWidget->setLockPanning(true);
  lockZoom_->setChecked(true);
  mGLWidget->setLockZoom(true);
  toggleLockColor(true);
}

void ViewSceneDialog::unlockAllTriggered()
{
  lockRotation_->setChecked(false);
  mGLWidget->setLockRotation(false);
  lockPan_->setChecked(false);
  mGLWidget->setLockPanning(false);
  lockZoom_->setChecked(false);
  mGLWidget->setLockZoom(false);
  toggleLockColor(false);
}

void ViewSceneDialog::setAutoRotateSpeed(double speed)
{
  auto spire = mSpire.lock();
  spire->setAutoRotateSpeed(speed);
}

void ViewSceneDialog::autoRotateRight()
{
  auto spire = mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(1.0, 0.0));
  pushCameraState();
}

void ViewSceneDialog::autoRotateLeft()
{
  auto spire = mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(-1.0, 0.0));
  pushCameraState();
}

void ViewSceneDialog::autoRotateUp()
{
  auto spire = mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(0.0, 1.0));
  pushCameraState();
}

void ViewSceneDialog::autoRotateDown()
{
  auto spire = mSpire.lock();
  spire->setAutoRotateVector(glm::vec2(0.0, -1.0));
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

static std::vector<WidgetHandle> filterGeomObjectsForWidgets(ViewScene::GeomListPtr geomData, ViewSceneControlsDock* mConfigurationDock)
{
  //getting geom list
  std::vector<WidgetHandle> objList;

  int port = 0;
  for (auto it = geomData->begin(); it != geomData->end(); ++it, ++port)
  {
    // Check if object is visible
    auto obj = *it; auto name = obj->uniqueID();
    auto displayName = QString::fromStdString(name).split(GeometryObject::delimiter).at(1);
    if (mConfigurationDock->visibleItems().isVisible(displayName))
    {
      auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
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
          objList.push_back(boost::dynamic_pointer_cast<WidgetBase>(realObj));
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
    Guard lock(Modules::Render::ViewScene::mutex_.get());
    LOG_DEBUG("ViewSceneDialog::asyncExecute after locking");

    auto spire = mSpire.lock();
    if (!spire) return;

    const auto geomData = getGeomData();
    if (geomData)
    {
      geomDataPresent = true;
      // Search for new widgets if geometry has changed
      const bool newGeometry = state_->getValue(Parameters::HasNewGeometry).toBool();
      if (newGeometry)
      {
        widgetHandles_ = filterGeomObjectsForWidgets(geomData, mConfigurationDock);
        state_->setValue(Parameters::HasNewGeometry, false);
      }

      // Search for new widget unless mouse and camera wasn't moved
      const auto adjustedX = x - mGLWidget->pos().x();
      const auto adjustedY = y - mGLWidget->pos().y();
      const auto currentCameraTransform = spire->getWorldToProjection();
      //TODO: extract function
      const bool reuseWidget = !newGeometry && previousWidgetInfo_.hasSameMousePosition(x, y)
        && previousWidgetInfo_.hasSameCameraTansform(currentCameraTransform);
      if (reuseWidget)
      {
        if (previousWidgetInfo_.hasPreviousWidget())
        {
          selectedWidget_ = previousWidgetInfo_.getPreviousWidget();
          spire->doInitialWidgetUpdate(selectedWidget_, adjustedX, adjustedY);
        }
        delayAfterLastSelection_ = 50;
      }
      else
      {
        spire->removeAllGeomObjects();
        spire->setWidgetInteractionMode(button);
        selectedWidget_ = spire->select(adjustedX, adjustedY, widgetHandles_);
        previousWidgetInfo_.setCameraTransform(currentCameraTransform);
        delayAfterLastSelection_ = 200;
      }

      if (selectedWidget_)
      {
        widgetColorChanger_ = boost::make_shared<ScopedWidgetColorChanger>(selectedWidget_, WidgetColor::RED);
        movementType_ = selectedWidget_->movementType(yetAnotherEnumConversion(button)).base;
        selectedWidget_->changeID();
      }
      previousWidgetInfo_.deletePreviousWidget();
    }
    previousWidgetInfo_.selectionAttempt();
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

  Guard lock(Modules::Render::ViewScene::mutex_.get());

  LOG_DEBUG("ViewSceneDialog::restoreObjColor after locking");

  widgetColorChanger_.reset();
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
  clippingPlaneManager_->setActive(0);

  const auto& activePlane = clippingPlaneManager_->active();
  mConfigurationDock->updatePlaneSettingsDisplay(
    activePlane.visible,
    activePlane.showFrame,
    activePlane.reverseNormal);
  mConfigurationDock->updatePlaneControlDisplay(
    activePlane.x,
    activePlane.y,
    activePlane.z,
    activePlane.d);
}

void ViewSceneDialog::setClippingPlaneIndex(int index)
{
  clippingPlaneManager_->setActive(index);

  doClippingPlanes();
}

void ViewSceneDialog::doClippingPlanes()
{
  const auto& activePlane = clippingPlaneManager_->active();
  mConfigurationDock->updatePlaneSettingsDisplay(
    activePlane.visible,
    activePlane.showFrame,
    activePlane.reverseNormal);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneVisible(bool value)
{
  clippingPlaneManager_->setActiveVisibility(value);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneFrameOn(bool value)
{
  updateModifiedGeometries();
  clippingPlaneManager_->setActiveFrameOn(value);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::reverseClippingPlaneNormal(bool value)
{
  clippingPlaneManager_->setActiveNormalReversed(value);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneX(int index)
{
  clippingPlaneManager_->setActiveX(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneY(int index)
{
  clippingPlaneManager_->setActiveY(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneZ(int index)
{
  clippingPlaneManager_->setActiveZ(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneD(int index)
{
  clippingPlaneManager_->setActiveD(index);
  updateClippingPlaneDisplay();
}

void ViewSceneDialog::updateClippingPlaneDisplay()
{
  newGeometryValue(false, true);

  delayGC_ = true;
  if (!delayedGCRequested_)
  {
    delayedGCRequested_ = true;
    runDelayedGC();
  }
}

void ViewSceneDialog::buildGeomClippingPlanes()
{
  auto spire = mSpire.lock();
  if (!spire)
    return;
  auto* clippingPlanes = spire->getClippingPlanes();
  if (!clippingPlanes)
    return;

  clippingPlaneGeoms_.clear();
  int index = 0;
  const auto& allPlanes = clippingPlaneManager_->allPlanes();
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
  renState.defaultColor = ColorRGB(clippingPlaneColors_[index][0], clippingPlaneColors_[index][1], clippingPlaneColors_[index][2]);
  renState.set(RenderState::ActionFlags::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::ActionFlags::USE_NORMALS, true);
  renState.set(RenderState::ActionFlags::IS_WIDGET, true);
  auto geom(boost::make_shared<GeometryObjectSpire>(*gid_, uniqueNodeID, false));
  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::ActionFlags::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, BBox(Point{}, Point{}), false, nullptr);

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
  auto geom2(boost::make_shared<GeometryObjectSpire>(*gid_, ss.str(), false));
  glyphs2.buildObject(*geom2, uniqueNodeID, renState.get(RenderState::ActionFlags::USE_TRANSPARENCY), 0.2,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, BBox(Point{}, Point{}), false, nullptr);

  clippingPlaneGeoms_.push_back(geom);
  clippingPlaneGeoms_.push_back(geom2);
}



//--------------------------------------------------------------------------------------------------
//---------------- Orientation Glyph ----------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::initializeAxes()
{
  auto spire = mSpire.lock();

  {
    bool visible = state_->getValue(Parameters::AxesVisible).toBool();
    mConfigurationDock->orientationCheckBox_->setChecked(visible);
    spire->showOrientation(visible);
  }

  {
    int axesSize = state_->getValue(Parameters::AxesSize).toInt();
    spire->setOrientSize(axesSize);
    ScopedWidgetSignalBlocker swsb(mConfigurationDock->orientAxisSize_);
    mConfigurationDock->orientAxisSize_->setValue(axesSize);
  }

  {
    int axesX = state_->getValue(Parameters::AxesX).toInt();
    spire->setOrientPosX(axesX);
    ScopedWidgetSignalBlocker swsb(mConfigurationDock->orientAxisXPos_);
    mConfigurationDock->orientAxisXPos_->setValue(axesX);
  }

  {
    int axesY = state_->getValue(Parameters::AxesY).toInt();
    spire->setOrientPosY(axesY);
    ScopedWidgetSignalBlocker swsb(mConfigurationDock->orientAxisYPos_);
    mConfigurationDock->orientAxisYPos_->setValue(axesY);
  }
}

void ViewSceneDialog::showOrientationChecked(bool value)
{
  auto spire = mSpire.lock();
  spire->showOrientation(value);
  state_->setValue(Parameters::AxesVisible, value);
}

void ViewSceneDialog::setOrientAxisSize(int value)
{
  auto spire = mSpire.lock();
  spire->setOrientSize(value);
  state_->setValue(Parameters::AxesSize, value);
}

void ViewSceneDialog::setOrientAxisPosX(int pos)
{
  auto spire = mSpire.lock();
  spire->setOrientPosX(pos);
  state_->setValue(Parameters::AxesX, pos);
}

void ViewSceneDialog::setOrientAxisPosY(int pos)
{
  auto spire = mSpire.lock();
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
  scaleBar_.visible = value;
  state_->setValue(Parameters::ShowScaleBar, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarFontSize(int value)
{
  scaleBar_.fontSize = value;
  state_->setValue(Parameters::ScaleBarFontSize, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarUnitValue(const QString& text)
{
  scaleBar_.unit = text.toStdString();
  state_->setValue(Parameters::ScaleBarUnitValue, text.toStdString());
  setScaleBar();
}

void ViewSceneDialog::setScaleBarLength(double value)
{
  scaleBar_.length = value;
  state_->setValue(Parameters::ScaleBarLength, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarHeight(double value)
{
  scaleBar_.height = value;
  state_->setValue(Parameters::ScaleBarHeight, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarMultiplier(double value)
{
  scaleBar_.multiplier = value;
  state_->setValue(Parameters::ScaleBarMultiplier, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarNumTicks(int value)
{
  scaleBar_.numTicks = value;
  state_->setValue(Parameters::ScaleBarNumTicks, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarLineWidth(double value)
{
  scaleBar_.lineWidth = value;
  state_->setValue(Parameters::ScaleBarLineWidth, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBar()
{
  if (scaleBar_.visible)
  {
    updateScaleBarLength();
    scaleBarGeom_ = buildGeometryScaleBar();
  }
  updateModifiedGeometries();
}

void ViewSceneDialog::updateScaleBarLength()
{
  auto spire = mSpire.lock();
  if (spire)
  {
    const auto width = spire->getScreenWidthPixels();
    const auto height = spire->getScreenHeightPixels();

    glm::vec4 p1(-scaleBar_.length / 2.0, 0.0, 0.0, 1.0);
    glm::vec4 p2(scaleBar_.length / 2.0, 0.0, 0.0, 1.0);
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
    scaleBar_.projLength = length(pp);
  }
}

GeometryHandle ViewSceneDialog::buildGeometryScaleBar()
{
  const int numTicks = scaleBar_.numTicks;
  double length = scaleBar_.projLength;
  const double height = scaleBar_.height;

  //figure out text length first
  const auto text_size = static_cast<size_t>(scaleBar_.fontSize);

  textBuilder_.initialize(text_size);

  //text
  std::stringstream ss;
  ss << scaleBar_.length * scaleBar_.multiplier << " " << scaleBar_.unit;
  auto oneline = ss.str();
  double text_len = 0.0;
  if (textBuilder_.isReady())
    text_len = std::get<0>(textBuilder_.getStringDims(oneline));
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

  std::shared_ptr<spire::VarBuffer> iboBufferSPtr(
    new spire::VarBuffer(vboSize));
  std::shared_ptr<spire::VarBuffer> vboBufferSPtr(
    new spire::VarBuffer(iboSize));

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
  ss << "_scaleBar::" << scaleBar_.fontSize << scaleBar_.length << scaleBar_.height << scaleBar_.numTicks << scaleBar_.projLength;
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
  uniforms.emplace_back("uColor", glm::vec4(1.0));
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

  auto geom(boost::make_shared<GeometryObjectSpire>(*gid_, uniqueNodeID, false));

  geom->ibos().push_back(geomIBO);
  geom->vbos().push_back(geomVBO);
  geom->passes().push_back(pass);

  //text
  if (textBuilder_.isReady())
  {
    if (textBuilder_.getFaceSize() != text_size)
      textBuilder_.setFaceSize(text_size);
    textBuilder_.setColor(1.0, 1.0, 1.0, 1.0);
    Vector shift(1.9, 0.1, 0.0);
    Vector trans(-text_len + 5, 0.0, 0.0);
    textBuilder_.printString(oneline, shift, trans, uniqueNodeID, *geom);
  }

  return geom;
}

//--------------------------------------------------------------------------------------------------
//---------------- Lights --------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::setLightColor(int index)
{
  const auto lightColor(mConfigurationDock->getLightColor(index));
  switch (index)
  {
  case 0:
    state_->setValue(Parameters::HeadLightColor, ColorRGB(lightColor.redF(), lightColor.greenF(), lightColor.blueF()).toString());
    break;
  case 1:
    state_->setValue(Parameters::Light1Color, ColorRGB(lightColor.redF(), lightColor.greenF(), lightColor.blueF()).toString());
    break;
  case 2:
    state_->setValue(Parameters::Light2Color, ColorRGB(lightColor.redF(), lightColor.greenF(), lightColor.blueF()).toString());
    break;
  case 3:
    state_->setValue(Parameters::Light3Color, ColorRGB(lightColor.redF(), lightColor.greenF(), lightColor.blueF()).toString());
    break;
  default:
    return;
  }

  auto spire = mSpire.lock();
  if (spire)
    spire->setLightColor(index, lightColor.redF(), lightColor.greenF(), lightColor.blueF());
}

void ViewSceneDialog::toggleHeadLight(bool value)
{
  toggleLightOnOff(0, value);
}

const static float PI = 3.1415926f;

void ViewSceneDialog::setHeadLightAzimuth(int value)
{
  state_->setValue(Parameters::HeadLightAzimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(0, value / 180.0f * PI - PI);
}

void ViewSceneDialog::setHeadLightInclination(int value)
{
  state_->setValue(Parameters::HeadLightInclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(0, value / 180.0f * PI - PI / 2.0f);
}

void ViewSceneDialog::toggleLight1(bool value)
{
  toggleLightOnOff(1, value);
}

void ViewSceneDialog::setLight1Azimuth(int value)
{
  state_->setValue(Parameters::Light1Azimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(1, value / 180.0f * PI - PI);
}

void ViewSceneDialog::setLight1Inclination(int value)
{
  state_->setValue(Parameters::Light1Inclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(1, value / 180.0f * PI - PI / 2.0f);
}

void ViewSceneDialog::toggleLight2(bool value)
{
  toggleLightOnOff(2, value);
}

void ViewSceneDialog::setLight2Azimuth(int value)
{
  state_->setValue(Parameters::Light2Azimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(2, value / 180.0f * PI - PI);
}

void ViewSceneDialog::setLight2Inclination(int value)
{
  state_->setValue(Parameters::Light2Inclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(2, value / 180.0f * PI - PI / 2.0f);
}

void ViewSceneDialog::toggleLight3(bool value)
{
  toggleLightOnOff(3, value);
}

void ViewSceneDialog::setLight3Azimuth(int value)
{
  state_->setValue(Parameters::Light3Azimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(3, value / 180.0f * PI - PI);
}

void ViewSceneDialog::setLight3Inclination(int value)
{
  state_->setValue(Parameters::Light3Inclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(3, value / 180.0f * PI - PI / 2.0f);
}

void ViewSceneDialog::toggleLightOnOff(int index, bool value)
{
  switch (index)
  {
  case 0:
    state_->setValue(Parameters::HeadLightOn, value);
    break;
  case 1:
    state_->setValue(Parameters::Light1On, value);
    break;
  case 2:
    state_->setValue(Parameters::Light2On, value);
    break;
  case 3:
    state_->setValue(Parameters::Light3On, value);
    break;
  default:
    return;
  }

  auto spire = mSpire.lock();
  if (spire)
    spire->setLightOn(index, value);
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
  if (value)
    setFog(FogFactor::FOG_INTENSITY, 1.0);
  else
    setFog(FogFactor::FOG_INTENSITY, 0.0);
  updateAllGeometries();
}

void ViewSceneDialog::setFogUseBGColor(bool value)
{
  state_->setValue(Parameters::UseBGColor, value);
  if (value)
    setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
  else
    setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
  updateAllGeometries();
}

void ViewSceneDialog::assignFogColor()
{
  const auto title = windowTitle() + " Choose fog color";
  const auto newColor = QColorDialog::getColor(fogColor_, this, title);
  if (newColor.isValid())
  {
    fogColor_ = newColor;
    mConfigurationDock->setFogColorLabel(fogColor_);
    state_->setValue(Parameters::FogColor, ColorRGB(fogColor_.red(), fogColor_.green(), fogColor_.blue()).toString());
  }
  bool useBg = state_->getValue(Parameters::UseBGColor).toBool();
  if (!useBg)
  {
    setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
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
  auto spire = mSpire.lock();
  if (spire)
    spire->setMaterialFactor(factor, value);
}

void ViewSceneDialog::setFog(FogFactor factor, double value)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setFog(factor, value);
}

void ViewSceneDialog::setFogColor(const glm::vec4 &color)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setFogColor(color/255.0f);
}

//--------------------------------------------------------------------------------------------------
//---------------- Misc. ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

void ViewSceneDialog::assignBackgroundColor()
{
  const auto title = windowTitle() + " Choose background color";
  const auto newColor = QColorDialog::getColor(bgColor_, this, title);
  if (newColor.isValid())
  {
    bgColor_ = newColor;
    mConfigurationDock->setSampleColor(bgColor_);
    state_->setValue(Parameters::BackgroundColor, ColorRGB(bgColor_.red(), bgColor_.green(), bgColor_.blue()).toString());
    auto spire = mSpire.lock();
    spire->setBackgroundColor(bgColor_);
    bool useBg = state_->getValue(Parameters::UseBGColor).toBool();
    if (useBg)
      setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
    else
      setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
    updateAllGeometries();
  }
}

void ViewSceneDialog::setTransparencySortTypeContinuous(bool)
{
  auto spire = mSpire.lock();
  spire->setTransparencyRenderType(RenderState::TransparencySortType::CONTINUOUS_SORT);
  updateAllGeometries();
}

void ViewSceneDialog::setTransparencySortTypeUpdate(bool)
{
  auto spire = mSpire.lock();
  spire->setTransparencyRenderType(RenderState::TransparencySortType::UPDATE_SORT);
  updateAllGeometries();
}

void ViewSceneDialog::setTransparencySortTypeLists(bool)
{
  auto spire = mSpire.lock();
  spire->setTransparencyRenderType(RenderState::TransparencySortType::LISTS_SORT);
  updateAllGeometries();
}

void ViewSceneDialog::screenshotClicked()
{
  takeScreenshot();
  screenshotTaker_->saveScreenshot();
}

void ViewSceneDialog::quickScreenshotClicked()
{
  takeScreenshot();
  screenshotTaker_->saveScreenshotFromPath();
}

void ViewSceneDialog::autoSaveScreenshot()
{
  QThread::sleep(1);
  takeScreenshot();
  const auto file = Screenshot::screenshotDirectory() +
                    QString("/%1_%2.png")
                    .arg(windowTitle().replace(':', '-'))
                    .arg(QTime::currentTime().toString("hh.mm.ss.zzz"));

  screenshotTaker_->saveScreenshot(file);
}

void ViewSceneDialog::sendBugReport()
{
  const QString glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  const QString gpuVersion = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

  // Temporarily save screenshot so that it can be sent over email
  takeScreenshot();
  QImage image = screenshotTaker_->getScreenshot();
  QString location = Screenshot::screenshotDirectory() + ("/scirun_bug.png");
  image.save(location);

  // Generate email template
  const QString askForScreenshot = "\nIMPORTANT: Make sure to attach the screenshot of the ViewScene located at "
                                   % location % "\n\n\n";
  static QString instructions = "## For bugs, follow the template below: fill out all pertinent sections,"
    "then delete the rest of the template to reduce clutter."
    "\n### If the prerequisite is met, just delete that text as well. "
    "If they're not all met, the issue will be closed or assigned back to you.\n\n";
  static QString prereqs = "**Prerequisite**\n* [ ] Did you [perform a cursory search](https://github.com/SCIInstitute/SCIRun/issues)"
    "to see if your bug or enhancement is already reported?\n\n";
  static QString reportGuide = "For more information on how to write a good "
    "[bug report](https://github.com/atom/atom/blob/master/CONTRIBUTING.md#how-do-i-submit-a-good-bug-report) or"
    "[enhancement request](https://github.com/atom/atom/blob/master/CONTRIBUTING.md#how-do-i-submit-a-good-enhancement-suggestion),"
    "see the `CONTRIBUTING` guide. These links point to another project, but most of the advice holds in general.\n\n";
  static QString describe = "**Describe the bug**\nA clear and concise description of what the bug is.\n\n";
  static QString askForData = "**Providing sample network(s) along with input data is useful to solving your issue.**\n\n";
  static QString reproduction = "**To Reproduce**\nSteps to reproduce the behavior:"
    "\n1. Go to '...'\n2. Click on '....'\n3. Scroll down to '....'\n4. See error\n\n";

  static QString expectedBehavior = "**Expected behavior**\nA clear and concise description of what you expected to happen.\n\n";
  static QString additional = "**Additional context**\nAdd any other context about the problem here.\n\n";
  const QString desktopInfo = "Desktop: " % QSysInfo::prettyProductName() % "\n";
  const QString kernelInfo = "Kernel: " % QSysInfo::kernelVersion() % "\n";
  const QString gpuInfo = "GPU: " % gpuVersion % "\n";

#ifndef OLDER_QT_SUPPORT_NEEDED // disable for older Qt 5 versions
  const QString qtInfo = "QT Version: " % QLibraryInfo::version().toString() % "\n";
  const QString glInfo = "GL Version: " % glVersion % "\n";
  const QString scirunVersionInfo = "SCIRun Version: " % QString::fromStdString(VersionInfo::GIT_VERSION_TAG) % "\n";
  const QString machineIdInfo = "Machine ID: " % QString(QSysInfo::machineUniqueId()) % "\n";

  //TODO: need generic email
  static QString recipient = "dwhite@sci.utah.edu";
  static QString subject = "View%20Scene%20Bug%20Report";
  QDesktopServices::openUrl(QUrl(QString("mailto:" % recipient % "?subject=" % subject % "&body=" %
                                         askForScreenshot % instructions % prereqs % reportGuide %
                                         describe % askForData % reproduction % expectedBehavior %
                                         additional % desktopInfo % kernelInfo % gpuInfo %
                                         qtInfo % glInfo % scirunVersionInfo % machineIdInfo)));
#endif
}

void ViewSceneDialog::takeScreenshot()
{
  if (!screenshotTaker_)
    screenshotTaker_ = new Screenshot(mGLWidget, this);

  screenshotTaker_->takeScreenshot();
}

void ViewSceneDialog::saveNewGeometryChanged(int state)
{
  saveScreenshotOnNewGeometry_ = state != 0;
}

void ViewSceneDialog::sendScreenshotDownstreamForTesting()
{
  takeScreenshot();
  state_->setTransientValue(Parameters::ScreenshotData, screenshotTaker_->toMatrix(), false);
}

void ViewSceneDialog::initializeVisibleObjects()
{
  mConfigurationDock->visibleItems().initializeSavedStateMap();
}
