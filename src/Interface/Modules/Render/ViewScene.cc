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

#include <es-log/trace-log.h>
#include <gl-platform/GLPlatform.hpp>

#include <Interface/Modules/Render/ViewScenePlatformCompatibility.h>
#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/GLWidget.h>
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Modules/Render/ViewScene.h>
#include <Interface/Modules/Render/Screenshot.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/GeometryPrimitives/Transform.h>
#include <boost/timer.hpp>

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
using namespace SCIRun::Modules::Render;

namespace
{
  class DialogIdGenerator : public GeometryIDGenerator
  {
  public:
    explicit DialogIdGenerator(const std::string& name) : moduleName_(name) {}
    virtual std::string generateGeometryID(const std::string& tag) const override
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

//--------------------------------------------------------------------------------------------------
ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state, QWidget* parent) :
  ModuleDialogGeneric(state, parent),
  gid_(new DialogIdGenerator(name))
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  setFocusPolicy(Qt::StrongFocus);

  setupScaleBar();
  addToolBar();
  setupClippingPlanes();

  // Setup Qt OpenGL widget.
  QGLFormat fmt;
  fmt.setAlpha(false);
  fmt.setRgba(true);
  fmt.setDepth(true);
  fmt.setDoubleBuffer(true);
  fmt.setDepthBufferSize(24);

  mGLWidget = new GLWidget(new QtGLContext(fmt), parentWidget());
  connect(mGLWidget, SIGNAL(fatalError(const QString&)), this, SIGNAL(fatalError(const QString&)));
  connect(this, SIGNAL(mousePressSignalForTestingGeometryObjectFeedback(int, int, const std::string&)), this, SLOT(sendGeometryFeedbackToState(int, int, const std::string&)));

  if (mGLWidget->isValid())
  {
    // Hook up the GLWidget
    glLayout->addWidget(mGLWidget);
    glLayout->update();

    // Set spire transient value (should no longer be used).
    mSpire = std::weak_ptr<SRInterface>(mGLWidget->getSpire());
  }
  else
  {
    /// \todo Display dialog.
    delete mGLWidget;
  }

  {
    auto spire = mSpire.lock();
    if (!spire)
      return;
    if (Preferences::Instance().useNewViewSceneMouseControls)
    {
      spire->setMouseMode(SRInterface::MOUSE_NEWSCIRUN);
      spire->setZoomInverted(Preferences::Instance().invertMouseZoom);
    }
    else
    {
      spire->setMouseMode(SRInterface::MOUSE_OLDSCIRUN);
    }
  }

  {
    //Set background Color
    auto colorStr = state_->getValue(Modules::Render::ViewScene::BackgroundColor).toString();
    bgColor_ = checkColorSetting(colorStr, Qt::black);

    auto spire = mSpire.lock();
    spire->setBackgroundColor(bgColor_);
  }

  setInitialLightValues();

  state->connectStateChanged([this]() { Q_EMIT newGeometryValueForwarder(); });
  connect(this, SIGNAL(newGeometryValueForwarder()), this, SLOT(updateModifiedGeometries()));

  std::string filesystemRoot = Application::Instance().executablePath().string();
  std::string sep;
  sep += boost::filesystem::path::preferred_separator;
  Modules::Visualization::TextBuilder::setFSStrings(filesystemRoot, sep);

  resizeTimer_.setSingleShot(true);
  connect(&resizeTimer_, SIGNAL(timeout()), this, SLOT(resizingDone()));
  resize(1000, 1000);
}

//--------------------------------------------------------------------------------------------------
std::string ViewSceneDialog::toString(std::string prefix) const
{
  auto spire = mSpire.lock();

  std::string output = "VIEW_SCENE:\n";
  prefix += "  ";

  output += prefix + "State:\n";
  output += "\n";

  output += spire->toString(prefix);
  output += "\n";

  return output;
}



//--------------------------------------------------------------------------------------------------
//---------------- Intitilization ------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addToolBar()
{
  mToolBar = new QToolBar(this);
  WidgetStyleMixin::toolbarStyle(mToolBar);

  addConfigurationButton();
  addConfigurationDock();
  addAutoViewButton();
  addScreenshotButton();

  glLayout->addWidget(mToolBar);

  addViewBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addConfigurationButton()
{
  QPushButton* configurationButton = new QPushButton();
  configurationButton->setToolTip("Open/Close Configuration Menu");
  configurationButton->setIcon(QPixmap(":/general/Resources/ViewScene/configure.png"));
  configurationButton->setShortcut(Qt::Key_F5);
  connect(configurationButton, SIGNAL(clicked(bool)), this, SLOT(configurationButtonClicked()));
  addToolbarButton(configurationButton);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addToolbarButton(QPushButton* button)
{
  button->setFixedSize(35,35);
  button->setIconSize(QSize(25,25));
  mToolBar->addWidget(button);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addConfigurationDock()
{
  QString name = windowTitle() + " Configuration";
  mConfigurationDock = new ViewSceneControlsDock(name, this);
  mConfigurationDock->setHidden(true);
  mConfigurationDock->setVisible(false);

  mConfigurationDock->setSampleColor(bgColor_);
  mConfigurationDock->setScaleBarValues(scaleBar_.visible, scaleBar_.fontSize, scaleBar_.length, scaleBar_.height,
    scaleBar_.multiplier, scaleBar_.numTicks, scaleBar_.visible, QString::fromStdString(scaleBar_.unit));
  setupMaterials();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setupMaterials()
{
  auto colorStr = state_->getValue(Modules::Render::ViewScene::FogColor).toString();
  if (!colorStr.empty())
  {
    ColorRGB color(colorStr);
    fogColor_ = QColor(static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
      static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
      static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));

    mConfigurationDock->setMaterialTabValues(
      state_->getValue(Modules::Render::ViewScene::Ambient).toDouble(),
      state_->getValue(Modules::Render::ViewScene::Diffuse).toDouble(),
      state_->getValue(Modules::Render::ViewScene::Specular).toDouble(),
      state_->getValue(Modules::Render::ViewScene::Shine).toDouble(),
      state_->getValue(Modules::Render::ViewScene::Emission).toDouble(),
      state_->getValue(Modules::Render::ViewScene::FogOn).toBool(),
      state_->getValue(Modules::Render::ViewScene::ObjectsOnly).toBool(),
      state_->getValue(Modules::Render::ViewScene::UseBGColor).toBool(),
      state_->getValue(Modules::Render::ViewScene::FogStart).toDouble(),
      state_->getValue(Modules::Render::ViewScene::FogEnd).toDouble());
  }
  else
  {
    fogColor_ = Qt::blue;
    mConfigurationDock->setMaterialTabValues(0.2, 1.0, 0.0, 1.0, 0.0, false, true, true, 0.0, 0.71);
  }
  mConfigurationDock->setFogColorLabel(fogColor_);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addAutoViewButton()
{
  autoViewButton_ = new QPushButton(this);
  autoViewButton_->setToolTip("Auto View");
  autoViewButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  autoViewButton_->setShortcut(Qt::Key_0);
  connect(autoViewButton_, SIGNAL(clicked(bool)), this, SLOT(autoViewClicked()));
  addToolbarButton(autoViewButton_);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addScreenshotButton()
{
  QPushButton* screenshotButton = new QPushButton(this);
  screenshotButton->setToolTip("Take screenshot");
  screenshotButton->setIcon(QPixmap(":/general/Resources/ViewScene/screenshot.png"));
  screenshotButton->setShortcut(Qt::Key_F12);
  connect(screenshotButton, SIGNAL(clicked(bool)), this, SLOT(screenshotClicked()));
  addToolbarButton(screenshotButton);
}

//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
using V = glm::vec3;
using P = std::tuple<V, V>;
using InnerMap = std::map<QString, P>;
static std::map<QString, InnerMap> axisViewParams;

static void initAxisViewParams()
{
  axisViewParams["+X"] = InnerMap {
    { "+Y", P(V(-1, 0, 0), V( 0, 1, 0)) },
    { "-Y", P(V( 1, 0, 0), V( 0,-1, 0)) },
    { "+Z", P(V( 0, 1, 0), V( 1, 0, 0)) },
    { "-Z", P(V( 0,-1, 0), V(-1, 0, 0)) }
  };
  axisViewParams["-X"] = InnerMap {
    { "+Y", P(V( 1, 0, 0), V( 0, 1, 0)) },
    { "-Y", P(V(-1, 0, 0), V( 0,-1, 0)) },
    { "+Z", P(V( 0, 1, 0), V(-1, 0, 0)) },
    { "-Z", P(V( 0,-1, 0), V( 1, 0, 0)) }
  };
  axisViewParams["+Y"] = InnerMap {
    { "+X", P(V( 1, 0, 0), V(0, 0, 1)) },
    { "-X", P(V(-1, 0, 0), V(0, 0, 1)) },
    { "+Z", P(V( 0, 1, 0), V(0, 0, 1)) },
    { "-Z", P(V( 0,-1, 0), V(0, 0, 1)) }
  };
  axisViewParams["-Y"] = InnerMap {
    { "+X", P(V(-1, 0, 0), V(0, 0,-1)) },
    { "-X", P(V( 1, 0, 0), V(0, 0,-1)) },
    { "+Z", P(V( 0, 1, 0), V(0, 0,-1)) },
    { "-Z", P(V( 0,-1, 0), V(0, 0,-1)) }
  };
  axisViewParams["+Z"] = InnerMap {
    { "+Y", P(V(0, 0, 1), V( 0, 1, 0)) },
    { "-Y", P(V(0, 0, 1), V( 0,-1, 0)) },
    { "+X", P(V(0, 0, 1), V(-1, 0, 0)) },
    { "-X", P(V(0, 0, 1), V( 1, 0, 0)) }
  };
  axisViewParams["-Z"] = InnerMap {
    { "+Y", P(V(0, 0,-1), V( 0, 1, 0)) },
    { "-Y", P(V(0, 0,-1), V( 0,-1, 0)) },
    { "+X", P(V(0, 0,-1), V( 1, 0, 0)) },
    { "-X", P(V(0, 0,-1), V(-1, 0, 0)) }
  };
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addViewOptions()
{
  QLabel* axisLabel = new QLabel();
  axisLabel->setText("Look Down Axis: ");
  mViewBar->addWidget(axisLabel);

  mDownViewBox = new QComboBox();
  mDownViewBox->setMinimumHeight(25);
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

  QLabel* vectorLabel = new QLabel();
  vectorLabel->setText("Up Vector: ");
  mViewBar->addWidget(vectorLabel);

  mUpVectorBox = new QComboBox();
  mUpVectorBox->setMinimumHeight(25);
  mUpVectorBox->setToolTip("Vector pointing up");
  connect(mUpVectorBox, SIGNAL(activated(const QString&)), this, SLOT(viewVectorSelected(const QString&)));
  mViewBar->addWidget(mUpVectorBox);
  mViewBar->setMinimumHeight(35);
  initAxisViewParams();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::addViewBarButton()
{
  viewBarBtn_ = new QPushButton();
  viewBarBtn_->setToolTip("Show View Options");
  viewBarBtn_->setIcon(QPixmap(":/general/Resources/ViewScene/views.png"));
  connect(viewBarBtn_, SIGNAL(clicked(bool)), this, SLOT(viewBarButtonClicked()));
  addToolbarButton(viewBarBtn_);
}

//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setupClippingPlanes()
{
  const int numClippingPlanes = 6;
  for (int i = 0; i < numClippingPlanes; ++i)
  {
    ClippingPlane plane;
    plane.visible = false;
    plane.showFrame = false;
    plane.reverseNormal = false;
    plane.x = 0.0;
    plane.y = 0.0;
    plane.z = 0.0;
    plane.d = 0.0;
    clippingPlanes_.push_back(plane);
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setupScaleBar()
{
  if (state_->getValue(Modules::Render::ViewScene::ScaleBarUnitValue).toString() != "")
  {
    scaleBar_.visible = state_->getValue(Modules::Render::ViewScene::ShowScaleBar).toBool();
    scaleBar_.unit = state_->getValue(Modules::Render::ViewScene::ScaleBarUnitValue).toString();
    scaleBar_.length = state_->getValue(Modules::Render::ViewScene::ScaleBarLength).toDouble();
    scaleBar_.height = state_->getValue(Modules::Render::ViewScene::ScaleBarHeight).toDouble();
    scaleBar_.multiplier = state_->getValue(Modules::Render::ViewScene::ScaleBarMultiplier).toDouble();
    scaleBar_.numTicks = state_->getValue(Modules::Render::ViewScene::ScaleBarNumTicks).toInt();
    scaleBar_.lineWidth = state_->getValue(Modules::Render::ViewScene::ScaleBarLineWidth).toDouble();
    scaleBar_.fontSize = state_->getValue(Modules::Render::ViewScene::ScaleBarFontSize).toInt();
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

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setInitialLightValues()
{
  auto light0str = state_->getValue(Modules::Render::ViewScene::HeadLightColor).toString();
  QColor light0 = checkColorSetting(light0str, Qt::white);
  int headlightAzimuth = state_->getValue(Modules::Render::ViewScene::HeadLightAzimuth).toInt();
  int headlightInclination = state_->getValue(Modules::Render::ViewScene::HeadLightInclination).toInt();

  auto light1str = state_->getValue(Modules::Render::ViewScene::Light1Color).toString();
  QColor light1 = checkColorSetting(light1str, Qt::white);
  int light1Azimuth = state_->getValue(Modules::Render::ViewScene::Light1Azimuth).toInt();
  int light1Inclination = state_->getValue(Modules::Render::ViewScene::Light1Inclination).toInt();

  auto light2str = state_->getValue(Modules::Render::ViewScene::Light2Color).toString();
  QColor light2 = checkColorSetting(light2str, Qt::white);
  int light2Azimuth = state_->getValue(Modules::Render::ViewScene::Light2Azimuth).toInt();
  int light2Inclination = state_->getValue(Modules::Render::ViewScene::Light2Inclination).toInt();

  auto light3str = state_->getValue(Modules::Render::ViewScene::Light3Color).toString();
  QColor light3 = checkColorSetting(light3str, Qt::white);
  int light3Azimuth = state_->getValue(Modules::Render::ViewScene::Light2Azimuth).toInt();
  int light3Inclination = state_->getValue(Modules::Render::ViewScene::Light2Inclination).toInt();

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

    spire->setLightOn(0, state_->getValue(Modules::Render::ViewScene::HeadLightOn).toBool());
    spire->setLightOn(1, state_->getValue(Modules::Render::ViewScene::Light1On).toBool());
    spire->setLightOn(2, state_->getValue(Modules::Render::ViewScene::Light2On).toBool());
    spire->setLightOn(3, state_->getValue(Modules::Render::ViewScene::Light3On).toBool());
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::pullSpecial()
{
  auto show = state_->getValue(Modules::Render::ViewScene::ShowViewer).toBool();

  if (show && parentWidget())
    parentWidget()->show();

  pulledSavedVisibility_ = true;
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::adjustToolbar()
{
  adjustToolbarForHighResolution(mToolBar);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setupRenderTabValues()
{
  auto valueSet = state_->getValue(Modules::Render::ViewScene::Lighting).toString();
  if (!valueSet.empty())
  {
    mConfigurationDock->setRenderTabValues(
      state_->getValue(Modules::Render::ViewScene::Lighting).toBool(),
      state_->getValue(Modules::Render::ViewScene::ShowBBox).toBool(),
      state_->getValue(Modules::Render::ViewScene::UseClip).toBool(),
      state_->getValue(Modules::Render::ViewScene::BackCull).toBool(),
      state_->getValue(Modules::Render::ViewScene::DisplayList).toBool(),
      state_->getValue(Modules::Render::ViewScene::Stereo).toBool(),
      state_->getValue(Modules::Render::ViewScene::StereoFusion).toDouble(),
      state_->getValue(Modules::Render::ViewScene::PolygonOffset).toDouble(),
      state_->getValue(Modules::Render::ViewScene::TextOffset).toDouble(),
      state_->getValue(Modules::Render::ViewScene::FieldOfView).toInt());
  }
  else
  {
    mConfigurationDock->setRenderTabValues(true, false, true, false, false, false, 0.4, 0.0, 0.0, 20);
  }
}

//--------------------------------------------------------------------------------------------------
QColor ViewSceneDialog::checkColorSetting(std::string& rgb, QColor defaultColor)
{
  QColor newColor;
  if (!rgb.empty())
  {
    ColorRGB color(rgb);
    newColor = QColor(static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
      static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
      static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));
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
  //if a render parameter changes we must update all of the geometrys by removing and radding them.
  //This must be foreced because the IDs will not have changed
  newGeometryValue(true);
}

void ViewSceneDialog::updateModifiedGeometries()
{
  //if we are looking for a new geoetry the ID will have changed therefore we can find the
  //geometrys that have changed and only remove those
  newGeometryValue(false);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::newGeometryValue(bool forceAllObjectsToUpdate)
{
  DEBUG_LOG_LINE_INFO
  LOG_DEBUG("ViewSceneDialog::newGeometryValue {} before locking", windowTitle().toStdString());
  RENDERER_LOG_FUNCTION_SCOPE;
  Guard lock(Modules::Render::ViewScene::mutex_.get());

  auto spire = mSpire.lock();
  if (!spire)
    return;

  if(forceAllObjectsToUpdate)
    spire->removeAllGeomObjects();

  std::vector<QString> displayNames;
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

  for (auto& plane : clippingPlaneGeoms_)
    allGeoms.push_back(plane);

  auto showFieldStates = transient_value_cast<ShowFieldStatesMap>(state_->getTransientValue(Parameters::ShowFieldStates));
  displayNames = mConfigurationDock->visibleItems().synchronize(allGeoms, showFieldStates);

  int port = 0;
  for (auto it = allGeoms.begin(); it != allGeoms.end(); ++it, ++port)
  {
    auto obj = *it;
    auto name = displayNames[port];
    if (mConfigurationDock->visibleItems().isVisible(name))
    {
      auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
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
      auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
      if (realObj && !spire->hasObject(obj->uniqueID()))
      {
        DEBUG_LOG_LINE_INFO
        spire->handleGeomObject(realObj, port);
      }
    }
  }

  sendScreenshotDownstreamForTesting();

  if (saveScreenshotOnNewGeometry_)
    screenshotClicked();

  //TODO IMPORTANT: we need some call somewhere to clear the transient geometry list once spire/ES has received the list of objects. They take up lots of memory...
  //state_->setTransientValue(Parameters::GeomData, boost::shared_ptr<std::list<boost::shared_ptr<Core::Datatypes::GeometryObject>>>(), false);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::sendGeometryFeedbackToState(int x, int y, const std::string& selName)
{
  auto spire = mSpire.lock();
  auto trans = spire->getWidgetTransform().transform;

  ViewSceneFeedback vsf;
  vsf.transform = toSciTransform(trans);
  vsf.selectionName = selName;
  state_->setTransientValue(Parameters::GeometryFeedbackInfo, vsf);
}



//--------------------------------------------------------------------------------------------------
//---------------- Input ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
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
    state_->setValue(Modules::Render::ViewScene::ShowViewer, true);
  }

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ModuleDialogGeneric::showEvent(evt);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::hideEvent(QHideEvent* evt)
{
  mConfigurationDock->setVisible(false);

  if (pulledSavedVisibility_)
  {
    ScopedWidgetSignalBlocker ssb(this);
    state_->setValue(Modules::Render::ViewScene::ShowViewer, false);
  }

  ModuleDialogGeneric::hideEvent(evt);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::closeEvent(QCloseEvent *evt)
{
  // NOTE: At one point this was required because the renderer was
  // multi-threaded. It is likely we will run into the same issue in the
  // future. Kept for future reference.
  //glLayout->removeWidget(mGLWidget);
  mGLWidget->close();
  state_->setValue(Modules::Render::ViewScene::ShowViewer, isVisible());
  ModuleDialogGeneric::closeEvent(evt);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::viewBarButtonClicked()
{
  hideViewBar_ = !hideViewBar_;
  mViewBar->setHidden(hideViewBar_);
  QString color = hideViewBar_ ? "rgb(66,66,69)" : "lightGray";
  viewBarBtn_->setStyleSheet("QPushButton { background-color: " + color + "; }");
  mDownViewBox->setCurrentIndex(0);
  mUpVectorBox->clear();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::configurationButtonClicked()
{
  mConfigurationDock->setVisible(!mConfigurationDock->isVisible());
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::resizeEvent(QResizeEvent *event)
{
  resizeTimer_.start(400);
  ModuleDialogGeneric::resizeEvent(event);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::resizingDone()
{
  ViewSceneFeedback vsf;
  vsf.windowSize = std::make_tuple(size().width(), size().height());
  state_->setTransientValue(Parameters::GeometryFeedbackInfo, vsf);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::mousePressEvent(QMouseEvent* event)
{
  if (shiftdown_)
  {
    selectObject(event->x(), event->y());
    updateModifiedGeometries();
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::mouseReleaseEvent(QMouseEvent* event)
{
  if (selected_)
  {
    selected_ = false;
    auto selName = restoreObjColor();
    updateModifiedGeometries();
    Q_EMIT mousePressSignalForTestingGeometryObjectFeedback(event->x(), event->y(), selName);
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::mouseMoveEvent(QMouseEvent* event)
{
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::wheelEvent(QWheelEvent* event)
{
  if (scaleBar_.visible)
  {
    updateScaleBarLength();
    scaleBarGeom_ = buildGeometryScaleBar();
    updateModifiedGeometries();
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::keyPressEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    shiftdown_ = true;
    break;
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::keyReleaseEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    shiftdown_ = false;
    break;
  }
}



//--------------------------------------------------------------------------------------------------
//---------------- Camera --------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

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
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::viewVectorSelected(const QString& name)
{
  glm::vec3 up, view;
  std::tie(view, up) = axisViewParams[mDownViewBox->currentText()][name];

  std::shared_ptr<SRInterface> spire = mSpire.lock();

  spire->setView(view, up);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::autoViewClicked()
{
  auto spireLock = mSpire.lock();
  spireLock->doAutoView();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::menuMouseControlChanged(int index)
{
  auto spire = mSpire.lock();
  if (!spire)
    return;

  if (index == 0)
  {
    spire->setMouseMode(SRInterface::MOUSE_OLDSCIRUN);
    Preferences::Instance().useNewViewSceneMouseControls.setValue(false);
  }
  else
  {
    spire->setMouseMode(SRInterface::MOUSE_NEWSCIRUN);
    Preferences::Instance().useNewViewSceneMouseControls.setValue(true);
  }
  mConfigurationDock->updateZoomOptionVisibility();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::invertZoomClicked(bool value)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setZoomInverted(value);
  Preferences::Instance().invertMouseZoom.setValue(value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::adjustZoomSpeed(int value)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setZoomSpeed(value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFieldOfView(int value)
{
  state_->setValue(Modules::Render::ViewScene::FieldOfView, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::toggleLockColor(bool locked)
{
  QString color = locked ? "red" : "rgb(66,66,69)";
  controlLock_->setStyleSheet("QPushButton { background-color: " + color + "; }");
  autoViewButton_->setDisabled(locked);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::lockRotationToggled()
{
  mGLWidget->setLockRotation(lockRotation_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::lockPanningToggled()
{
  mGLWidget->setLockPanning(lockPan_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::lockZoomToggled()
{
  mGLWidget->setLockZoom(lockZoom_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::autoViewOnLoadChecked(bool value)
{
  //TODO: Add to SRInterface
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::useOrthoViewChecked(bool value)
{
  //TODO: Add to SRInterface
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::stereoChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::Stereo, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setStereoFusion(int value)
{
  double fusion = value / 100;
  state_->setValue(Modules::Render::ViewScene::StereoFusion, fusion);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setPolygonOffset(int value)
{
  double offset = value / 100;
  state_->setValue(Modules::Render::ViewScene::PolygonOffset, offset);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setTextOffset(int value)
{
  double offset = value / 100;
  state_->setValue(Modules::Render::ViewScene::TextOffset, offset);
}



//--------------------------------------------------------------------------------------------------
//---------------- Widgets -------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

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

//--------------------------------------------------------------------------------------------------
static std::list<GeometryHandle> filterGeomObjecsForWidgets(SCIRun::Modules::Render::ViewScene::GeomListPtr geomData, ViewSceneControlsDock* mConfigurationDock)
{
  //getting geom list
  std::list<GeometryHandle> objList;

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
          if (pass.renderState.get(SCIRun::RenderState::IS_WIDGET))
          {
            isWidget = true;
            break;
          }
        }
        if (isWidget)
          objList.push_back(realObj);
      }
    }
  }

  return objList;
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::selectObject(const int x, const int y)
{
  LOG_DEBUG("ViewSceneDialog::asyncExecute before locking");
  Guard lock(Modules::Render::ViewScene::mutex_.get());
  LOG_DEBUG("ViewSceneDialog::asyncExecute after locking");

  auto spire = mSpire.lock();
  if (!spire)
    return;

  spire->removeAllGeomObjects();

  // Grab the geomData transient value.
  auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
  if (geomDataTransient && !geomDataTransient->empty())
  {
    auto geomData = transient_value_cast<Modules::Render::ViewScene::GeomListPtr>(geomDataTransient);
    if (!geomData)
    {
      LOG_DEBUG("Logical error: ViewSceneDialog received an empty list.");
      return;
    }

    //get widgets
    std::list<GeometryHandle> objList = filterGeomObjecsForWidgets(geomData, mConfigurationDock);

    //select widget
    spire->select(glm::ivec2(x - mGLWidget->pos().x(), y - mGLWidget->pos().y()), objList, 0);

    std::string selName = spire->getSelection();
    if (selName != "")
    {
      for (auto &obj : objList)
      {
        if (obj->uniqueID() == selName)
        {
          selected_ = true;
          for (auto& pass : obj->passes())
          {
            pass.addUniform("uAmbientColor",
              glm::vec4(0.1f, 0.0f, 0.0f, 1.0f));
            pass.addUniform("uDiffuseColor",
              glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            pass.addUniform("uSpecularColor",
              glm::vec4(0.1f, 0.0f, 0.0f, 1.0f));
          }
          break;
        }
      }
    }
  }
}

//--------------------------------------------------------------------------------------------------
std::string ViewSceneDialog::restoreObjColor()
{
  LOG_DEBUG("ViewSceneDialog::asyncExecute before locking");

  Guard lock(Modules::Render::ViewScene::mutex_.get());

  LOG_DEBUG("ViewSceneDialog::asyncExecute after locking");

  auto spire = mSpire.lock();
  if (!spire)
    return "";

  std::string selName = spire->getSelection();
  if (!selName.empty())
  {
    auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
    if (geomDataTransient && !geomDataTransient->empty())
    {
      auto geomData = transient_value_cast<Modules::Render::ViewScene::GeomListPtr>(geomDataTransient);
      if (!geomData)
      {
        LOG_DEBUG("Logical error: ViewSceneDialog received an empty list.");
        return "";
      }
      for (auto it = geomData->begin(); it != geomData->end(); ++it)
      {
        auto obj = *it;
        auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
        if (realObj->uniqueID() == selName)
        {
          for (auto& pass : realObj->passes())
          {
            pass.addUniform("uAmbientColor",
              glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));
            pass.addUniform("uDiffuseColor",
              glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            pass.addUniform("uSpecularColor",
              glm::vec4(0.1f, 1.0f, 1.0f, 1.0f));
          }
          break;
        }
      }
    }
  }
  return selName;
}



//--------------------------------------------------------------------------------------------------
//---------------- Clipping Planes -----------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setClippingPlaneIndex(int index)
{
  clippingPlaneIndex_ = index;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneIndex(clippingPlaneIndex_);
  mConfigurationDock->updatePlaneSettingsDisplay(
    clippingPlanes_[clippingPlaneIndex_].visible,
    clippingPlanes_[clippingPlaneIndex_].showFrame,
    clippingPlanes_[clippingPlaneIndex_].reverseNormal);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setClippingPlaneVisible(bool value)
{
  clippingPlanes_[clippingPlaneIndex_].visible = value;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneVisible(clippingPlanes_[clippingPlaneIndex_].visible);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setClippingPlaneFrameOn(bool value)
{
  updateModifiedGeometries();
  clippingPlanes_[clippingPlaneIndex_].showFrame = value;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneFrameOn(clippingPlanes_[clippingPlaneIndex_].showFrame);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::reverseClippingPlaneNormal(bool value)
{
  clippingPlanes_[clippingPlaneIndex_].reverseNormal = value;
  auto spire = mSpire.lock();
  if (spire)
    spire->reverseClippingPlaneNormal(clippingPlanes_[clippingPlaneIndex_].reverseNormal);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setClippingPlaneX(int index)
{
  clippingPlanes_[clippingPlaneIndex_].x = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneX(clippingPlanes_[clippingPlaneIndex_].x);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setClippingPlaneY(int index)
{
  clippingPlanes_[clippingPlaneIndex_].y = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneY(clippingPlanes_[clippingPlaneIndex_].y);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setClippingPlaneZ(int index)
{
  clippingPlanes_[clippingPlaneIndex_].z = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneZ(clippingPlanes_[clippingPlaneIndex_].z);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setClippingPlaneD(int index)
{
  clippingPlanes_[clippingPlaneIndex_].d = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneD(clippingPlanes_[clippingPlaneIndex_].d);
  updatClippingPlaneDisplay();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::useClipChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::UseClip, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::updatClippingPlaneDisplay()
{
  mConfigurationDock->updatePlaneControlDisplay(
    clippingPlanes_[clippingPlaneIndex_].x,
    clippingPlanes_[clippingPlaneIndex_].y,
    clippingPlanes_[clippingPlaneIndex_].z,
    clippingPlanes_[clippingPlaneIndex_].d);

  //geometry
  buildGeomClippingPlanes();
  updateModifiedGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::buildGeomClippingPlanes()
{
  auto spire = mSpire.lock();
  if (!spire)
    return;
  auto clippingPlanes = spire->getClippingPlanes();

  clippingPlaneGeoms_.clear();
  int index = 0;
  for (auto i : clippingPlanes->clippingPlanes)
  {
    if (clippingPlanes_[index].showFrame)
      buildGeometryClippingPlane(index, i, spire->getSceneBox());
    index++;
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::buildGeometryClippingPlane(int index, glm::vec4 plane, const BBox& bbox)
{
  BBox mBBox;
  mBBox.reset();
  Vector diag(bbox.diagonal());
  Point c(bbox.center());
  Vector n(plane.x, plane.y, plane.z);
  n.normalize();
  auto p(c + ((-plane.w) - Dot(c, n)) * n);
  if (clippingPlanes_[index].reverseNormal)
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
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);
  renState.defaultColor = ColorRGB(0.4, 0.4, 1);
  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);
  auto geom(boost::make_shared<GeometryObjectSpire>(*gid_, uniqueNodeID, false));
  glyphs.buildObject(*geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, BBox());

  Graphics::GlyphGeom glyphs2;
  glyphs2.addPlane(p1, p2, p3, p4, ColorRGB());
  ss.str("");
  ss << "clipping_plane_trans" << index <<
    p1.x() << p1.y() << p1.z() <<
    p2.x() << p2.y() << p2.z() <<
    p3.x() << p3.y() << p3.z() <<
    p4.x() << p4.y() << p4.z();
  uniqueNodeID = ss.str();
  renState.set(RenderState::USE_TRANSPARENCY, true);
  renState.defaultColor = ColorRGB(1, 1, 1, 0.2);
  auto geom2(boost::make_shared<GeometryObjectSpire>(*gid_, ss.str(), false));
  glyphs2.buildObject(*geom2, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 0.2,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, BBox());

  clippingPlaneGeoms_.push_back(geom);
  clippingPlaneGeoms_.push_back(geom2);
}



//--------------------------------------------------------------------------------------------------
//---------------- Orietation Glyph ----------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::showOrientationChecked(bool value)
{
  auto spire = mSpire.lock();
  spire->showOrientation(value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setOrientAxisSize(int value)
{
  auto spire = mSpire.lock();
  spire->setOrientSize(value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setOrientAxisPosX(int pos)
{
  auto spire = mSpire.lock();
  spire->setOrientPosX(pos);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setOrientAxisPosY(int pos)
{
  auto spire = mSpire.lock();
  spire->setOrientPosY(pos);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setCenterOrientPos()
{
  setOrientAxisPosX(50);
  setOrientAxisPosY(50);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setDefaultOrientPos()
{
  setOrientAxisPosX(100);
  setOrientAxisPosY(100);
}



//--------------------------------------------------------------------------------------------------
//---------------- Scale Bar -----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarVisible(bool value)
{
  scaleBar_.visible = value;
  state_->setValue(Modules::Render::ViewScene::ShowScaleBar, value);
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarFontSize(int value)
{
  scaleBar_.fontSize = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarFontSize, value);
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarUnitValue(const QString& text)
{
  scaleBar_.unit = text.toStdString();
  state_->setValue(Modules::Render::ViewScene::ScaleBarUnitValue, text.toStdString());
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarLength(double value)
{
  scaleBar_.length = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarLength, value);
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarHeight(double value)
{
  scaleBar_.height = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarHeight, value);
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarMultiplier(double value)
{
  scaleBar_.multiplier = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarMultiplier, value);
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarNumTicks(int value)
{
  scaleBar_.numTicks = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarNumTicks, value);
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBarLineWidth(double value)
{
  scaleBar_.lineWidth = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarLineWidth, value);
  setScaleBar();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setScaleBar()
{
  if (scaleBar_.visible)
  {
    updateScaleBarLength();
    scaleBarGeom_ = buildGeometryScaleBar();
    updateModifiedGeometries();
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::updateScaleBarLength()
{
  auto spire = mSpire.lock();
  if (spire)
  {
    size_t width = spire->getScreenWidthPixels();
    size_t height = spire->getScreenHeightPixels();

    glm::vec4 p1(-scaleBar_.length / 2.0, 0.0, 0.0, 1.0);
    glm::vec4 p2(scaleBar_.length / 2.0, 0.0, 0.0, 1.0);
    glm::mat4 matIV = spire->getWorldToView();
    matIV[0][0] = 1.0; matIV[0][1] = 0.0; matIV[0][2] = 0.0;
    matIV[1][0] = 0.0; matIV[1][1] = 1.0; matIV[1][2] = 0.0;
    matIV[2][0] = 0.0; matIV[2][1] = 0.0; matIV[2][2] = 1.0;
    glm::mat4 matProj = spire->getViewToProjection();
    p1 = matProj * matIV * p1;
    p2 = matProj * matIV * p2;
    glm::vec2 p(p1.x / p1.w - p2.x / p2.w, p1.y / p1.w - p2.y / p2.w);
    glm::vec2 pp(p.x*width / 2.0,
      p.y*height / 2.0);
    scaleBar_.projLength = glm::length(pp);
  }
}

//--------------------------------------------------------------------------------------------------
GeometryHandle ViewSceneDialog::buildGeometryScaleBar()
{
  const int    numTicks = scaleBar_.numTicks;
  double length = scaleBar_.projLength;
  const double height = scaleBar_.height;
  glm::vec4 color(1.0);
  glm::vec4 shift(1.9, 0.1, 0.0, 0.0);

  //figure out text length first
  size_t text_size = static_cast<size_t>(scaleBar_.fontSize);

  textBuilder_.initialize(text_size);

  //text
  std::stringstream ss;
  std::string oneline;
  ss << scaleBar_.length * scaleBar_.multiplier << " " << scaleBar_.unit;
  oneline = ss.str();
  double text_len = 0.0;
  if (textBuilder_.isReady())
    text_len = std::get<0>(textBuilder_.getStringDims(oneline));
  text_len += 5;//add a 5-pixel gap

  std::vector<Vector> points;
  std::vector<uint32_t> indices;
  int32_t numVBOElements = 0;
  uint32_t index = 0;
  //base line
  points.push_back(Vector(-length - text_len, 0.0, 0.0));
  points.push_back(Vector(-text_len, 0.0, 0.0));
  numVBOElements += 2;
  indices.push_back(index++);
  indices.push_back(index++);
  if (numTicks > 1)
  {
    for (int i = 0; i < numTicks; ++i)
    {
      double x = -length - text_len + i*length / (numTicks - 1);
      points.push_back(Vector(x, 0.0, 0.0));
      points.push_back(Vector(x, height, 0.0));
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

  spire::VarBuffer* iboBuffer = iboBufferSPtr.get();
  spire::VarBuffer* vboBuffer = vboBufferSPtr.get();

  for (auto a : indices) iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++) {
    vboBuffer->write(static_cast<float>(points[i].x()));
    vboBuffer->write(static_cast<float>(points[i].y()));
    vboBuffer->write(static_cast<float>(points[i].z()));
  }

  ss.str("");
  ss << "_scaleBar::" << scaleBar_.fontSize << scaleBar_.length << scaleBar_.height << scaleBar_.numTicks << scaleBar_.projLength;
  auto uniqueNodeID = ss.str();
  auto vboName = uniqueNodeID + "VBO";
  auto iboName = uniqueNodeID + "IBO";
  auto passName = uniqueNodeID + "Pass";

  // Construct VBO.
  std::string shader = "Shaders/HudUniform";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  std::vector<SpireSubPass::Uniform> uniforms;
  uniforms.push_back(SpireSubPass::Uniform("uTrans", shift));
  uniforms.push_back(SpireSubPass::Uniform("uColor", color));
  SpireVBO geomVBO(vboName, attribs, vboBufferSPtr,
    numVBOElements, BBox(), true);

  // Construct IBO.

  SpireIBO geomIBO(iboName, SpireIBO::PRIMITIVE::LINES, sizeof(uint32_t), iboBufferSPtr);

  RenderState renState;
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::HAS_DATA, true);
  renState.set(RenderState::USE_COLORMAP, false);
  renState.set(RenderState::USE_TRANSPARENCY, false);
  renState.set(RenderState::IS_TEXT, true);

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

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setLightColor(int index)
{
  QColor lightColor(mConfigurationDock->getLightColor(index));
  switch (index)
  {
  case 0:
    state_->setValue(Modules::Render::ViewScene::HeadLightColor, ColorRGB(lightColor.red(), lightColor.green(), lightColor.blue()).toString());
    break;
  case 1:
    state_->setValue(Modules::Render::ViewScene::Light1Color, ColorRGB(lightColor.red(), lightColor.green(), lightColor.blue()).toString());
    break;
  case 2:
    state_->setValue(Modules::Render::ViewScene::Light2Color, ColorRGB(lightColor.red(), lightColor.green(), lightColor.blue()).toString());
    break;
  case 3:
    state_->setValue(Modules::Render::ViewScene::Light3Color, ColorRGB(lightColor.red(), lightColor.green(), lightColor.blue()).toString());
    break;
  default:
    return;
  }

  auto spire = mSpire.lock();
  if (spire)
    spire->setLightColor(index, lightColor.redF(), lightColor.greenF(), lightColor.blueF());
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::toggleHeadLight(bool value)
{
  toggleLightOnOff(0, value);
}

const static float PI = 3.1415926f;

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setHeadLightAzimuth(int value)
{
  state_->setValue(Modules::Render::ViewScene::HeadLightAzimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(0, value / 180.0f * PI - PI);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setHeadLightInclination(int value)
{
  state_->setValue(Modules::Render::ViewScene::HeadLightInclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(0, value / 180.0f * PI - PI / 2.0f);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::toggleLight1(bool value)
{
  toggleLightOnOff(1, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setLight1Azimuth(int value)
{
  state_->setValue(Modules::Render::ViewScene::Light1Azimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(1, value / 180.0f * PI - PI);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setLight1Inclination(int value)
{
  state_->setValue(Modules::Render::ViewScene::Light1Inclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(1, value / 180.0f * PI - PI / 2.0f);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::toggleLight2(bool value)
{
  toggleLightOnOff(2, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setLight2Azimuth(int value)
{
  state_->setValue(Modules::Render::ViewScene::Light2Azimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(2, value / 180.0f * PI - PI);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setLight2Inclination(int value)
{
  state_->setValue(Modules::Render::ViewScene::Light2Inclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(2, value / 180.0f * PI - PI / 2.0f);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::toggleLight3(bool value)
{
  toggleLightOnOff(3, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setLight3Azimuth(int value)
{
  state_->setValue(Modules::Render::ViewScene::Light3Azimuth, value);
  auto spire = mSpire.lock();
  spire->setLightAzimuth(3, value / 180.0f * PI - PI);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setLight3Inclination(int value)
{
  state_->setValue(Modules::Render::ViewScene::Light3Inclination, value);
  auto spire = mSpire.lock();
  spire->setLightInclination(3, value / 180.0f * PI - PI / 2.0f);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::lightingChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::Lighting, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::toggleLightOnOff(int index, bool value)
{
  switch (index)
  {
  case 0:
    state_->setValue(Modules::Render::ViewScene::HeadLightOn, value);
    break;
  case 1:
    state_->setValue(Modules::Render::ViewScene::Light1On, value);
    break;
  case 2:
    state_->setValue(Modules::Render::ViewScene::Light2On, value);
    break;
  case 3:
    state_->setValue(Modules::Render::ViewScene::Light3On, value);
    break;
  default:
    return;
  }

  auto spire = mSpire.lock();
  if (spire)
    spire->setLightOn(index, value);
}

void ViewSceneDialog::updateLightDirection(int light)
{

}



//--------------------------------------------------------------------------------------------------
//---------------- Materials -----------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setAmbientValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Ambient, value);
  setMaterialFactor(SRInterface::MAT_AMBIENT, value);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setDiffuseValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Diffuse, value);
  setMaterialFactor(SRInterface::MAT_DIFFUSE, value);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setSpecularValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Specular, value);
  setMaterialFactor(SRInterface::MAT_SPECULAR, value);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setShininessValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Shine, value);
  setMaterialFactor(SRInterface::MAT_SHINE, value);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setEmissionValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Emission, value);
}



//--------------------------------------------------------------------------------------------------
//---------------- Fog -----------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFogOn(bool value)
{
  state_->setValue(Modules::Render::ViewScene::FogOn, value);
  if (value)
    setFog(SRInterface::FOG_INTENSITY, 1.0);
  else
    setFog(SRInterface::FOG_INTENSITY, 0.0);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFogOnVisibleObjects(bool value)
{
  state_->setValue(Modules::Render::ViewScene::ObjectsOnly, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFogUseBGColor(bool value)
{
  state_->setValue(Modules::Render::ViewScene::UseBGColor, value);
  if (value)
    setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
  else
    setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::assignFogColor()
{
  QString title = windowTitle() + " Choose fog color";
  auto newColor = QColorDialog::getColor(fogColor_, this, title);
  if (newColor.isValid())
  {
    fogColor_ = newColor;
    mConfigurationDock->setFogColorLabel(fogColor_);
    state_->setValue(Modules::Render::ViewScene::FogColor, ColorRGB(fogColor_.red(), fogColor_.green(), fogColor_.blue()).toString());
  }
  bool useBg = state_->getValue(Modules::Render::ViewScene::UseBGColor).toBool();
  if (!useBg)
  {
    setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
    updateAllGeometries();
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFogStartValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::FogStart, value);
  setFog(SRInterface::FOG_START, value);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFogEndValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::FogEnd, value);
  setFog(SRInterface::FOG_END, value);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setMaterialFactor(int factor, double value)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setMaterialFactor(static_cast<SRInterface::MatFactor>(factor), value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFog(int factor, double value)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setFog(static_cast<SRInterface::FogFactor>(factor), value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setFogColor(const glm::vec4 &color)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setFogColor(color/255.0);
}



//--------------------------------------------------------------------------------------------------
//---------------- Misc. ---------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::assignBackgroundColor()
{
  QString title = windowTitle() + " Choose background color";
  auto newColor = QColorDialog::getColor(bgColor_, this, title);
  if (newColor.isValid())
  {
    bgColor_ = newColor;
    mConfigurationDock->setSampleColor(bgColor_);
    state_->setValue(Modules::Render::ViewScene::BackgroundColor, ColorRGB(bgColor_.red(), bgColor_.green(), bgColor_.blue()).toString());
    std::shared_ptr<SRInterface> spire = mSpire.lock();
    spire->setBackgroundColor(bgColor_);
    bool useBg = state_->getValue(Modules::Render::ViewScene::UseBGColor).toBool();
    if (useBg)
      setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
    else
      setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
    updateAllGeometries();
  }
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setTransparencySortTypeContinuous(bool index)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setTransparencyRendertype(RenderState::TransparencySortType::CONTINUOUS_SORT);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setTransparencySortTypeUpdate(bool index)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setTransparencyRendertype(RenderState::TransparencySortType::UPDATE_SORT);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::setTransparencySortTypeLists(bool index)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setTransparencyRendertype(RenderState::TransparencySortType::LISTS_SORT);
  updateAllGeometries();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::screenshotClicked()
{
  takeScreenshot();
  screenshotTaker_->saveScreenshot();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::takeScreenshot()
{
  if (!screenshotTaker_)
    screenshotTaker_ = new Screenshot(mGLWidget, this);

  screenshotTaker_->takeScreenshot();
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::saveNewGeometryChanged(int state)
{
  saveScreenshotOnNewGeometry_ = state != 0;
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::sendScreenshotDownstreamForTesting()
{
  takeScreenshot();
  state_->setTransientValue(Parameters::ScreenshotData, screenshotTaker_->toMatrix(), false);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::showBBoxChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::ShowBBox, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::useBackCullChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::BackCull, value);
}

//--------------------------------------------------------------------------------------------------
void ViewSceneDialog::displayListChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::DisplayList, value);
}
