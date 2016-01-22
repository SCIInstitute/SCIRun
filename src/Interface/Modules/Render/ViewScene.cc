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

#include <gl-platform/GLPlatform.hpp>

#include <Interface/Modules/Render/ViewScenePlatformCompatibility.h>
#include <Interface/Modules/Render/ES/SRInterface.h>
#include <Interface/Modules/Render/GLWidget.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Modules/Render/ViewScene.h>
#include <Interface/Modules/Render/Screenshot.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms::Render;
using namespace SCIRun::Render;
using namespace SCIRun::Modules::Render;

//------------------------------------------------------------------------------
ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), mConfigurationDock(nullptr), shown_(false), itemValueChanged_(true),
  screenshotTaker_(nullptr), saveScreenshotOnNewGeometry_(false), shiftdown_(false), selected_(false)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  setFocusPolicy(Qt::StrongFocus);

  addToolBar();

  // Setup Qt OpenGL widget.
  QGLFormat fmt;
  fmt.setAlpha(true);
  fmt.setRgba(true);
  fmt.setDepth(true);
  fmt.setDoubleBuffer(true);
  fmt.setDepthBufferSize(24);

  mGLWidget = new GLWidget(new QtGLContext(fmt), parentWidget());
  connect(mGLWidget, SIGNAL(fatalError(const QString&)), this, SIGNAL(fatalError(const QString&)));
  connect(mGLWidget, SIGNAL(mousePressSignalForTestingGeometryObjectFeedback(int, int)), this, SLOT(sendGeometryFeedbackToState(int, int)));

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
    if (Core::Preferences::Instance().useNewViewSceneMouseControls)
    {
      spire->setMouseMode(SRInterface::MOUSE_NEWSCIRUN);
      spire->setZoomInverted(Core::Preferences::Instance().invertMouseZoom);
    }
    else
    {
      spire->setMouseMode(SRInterface::MOUSE_OLDSCIRUN);
    }
  }

  {
    //Set background Color
    if (state_->getValue(Modules::Render::ViewScene::BackgroundColor).toString() != "")
    {
      ColorRGB color(state_->getValue(Modules::Render::ViewScene::BackgroundColor).toString());
      bgColor_ = QColor(static_cast<int>(color.r() > 1 ? color.r() : color.r() * 255.0),
        static_cast<int>(color.g() > 1 ? color.g() : color.g() * 255.0),
        static_cast<int>(color.b() > 1 ? color.b() : color.b() * 255.0));
    }
    else
    {
      bgColor_ = Qt::black;
    }
    auto spire = mSpire.lock();
    spire->setBackgroundColor(bgColor_);
  }

  state->connect_state_changed(boost::bind(&ViewSceneDialog::newGeometryValueForwarder, this));
  connect(this, SIGNAL(newGeometryValueForwarder()), this, SLOT(newGeometryValue()));
}

void ViewSceneDialog::mousePressEvent(QMouseEvent* event)
{
  if (shiftdown_)
  {
    selectObject(event->x(), event->y());
    newGeometryValue();
  }
}

void ViewSceneDialog::restoreObjColor()
{
  LOG_DEBUG("ViewSceneDialog::asyncExecute before locking");

  Guard lock(Modules::Render::ViewScene::mutex_.get());

  LOG_DEBUG("ViewSceneDialog::asyncExecute after locking");

  auto spire = mSpire.lock();
  if (!spire)
    return;

  std::string selName = spire->getSelection();
  if (selName != "")
  {
    auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
    if (geomDataTransient && !geomDataTransient->empty())
    {
      auto geomData = transient_value_cast<Modules::Render::ViewScene::GeomListPtr>(geomDataTransient);
      if (!geomData)
      {
        LOG_DEBUG("Logical error: ViewSceneDialog received an empty list.");
        return;
      }
      for (auto it = geomData->begin(); it != geomData->end(); ++it)
      {
        auto obj = *it;
        auto realObj = boost::dynamic_pointer_cast<Graphics::Datatypes::GeometryObjectSpire>(obj);
        if (realObj->uniqueID() == selName)
        {
          selected_ = true;
          for (auto& pass : realObj->mPasses)
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
}

void ViewSceneDialog::mouseReleaseEvent(QMouseEvent* event)
{
  if (selected_)
  {
    restoreObjColor();
    newGeometryValue();
    selected_ = false;
  }
}

void ViewSceneDialog::keyPressEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    shiftdown_ = true;
    break;
  }
}

void ViewSceneDialog::keyReleaseEvent(QKeyEvent* event)
{
  switch (event->key())
  {
  case Qt::Key_Shift:
    shiftdown_ = false;
    break;
  }
}

void ViewSceneDialog::selectObject(const int x, const int y)
{
  //newGeometryValue
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
    if (!spire)
    {
      LOG_DEBUG("Logical error: Spire lock not acquired.");
      return;
    }

    //getting geom list
    std::list<Graphics::Datatypes::GeometryHandle> objList;

    for (auto it = geomData->begin(); it != geomData->end(); ++it)
    {
      auto obj = *it;
      auto realObj = boost::dynamic_pointer_cast<Graphics::Datatypes::GeometryObjectSpire>(obj);
      if (realObj)
      {
        //filter objs
        bool isWidget = false;
        for (auto& pass : realObj->mPasses)
        {
          if (pass.renderState.get(RenderState::IS_WIDGET))
          {
            isWidget = true;
            break;
          }
        }
        if (isWidget)
          objList.push_back(realObj);
      }
    }

    spire->select(glm::ivec2(x - mGLWidget->pos().x(),
      y - mGLWidget->pos().y()), objList, 0);
    std::string selName = spire->getSelection();
    if (selName != "")
    {
      for (auto &obj : objList)
      {
        if (obj->uniqueID() == selName)
        {
          selected_ = true;
          for (auto& pass : obj->mPasses)
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
  else
  {
    if (!spire)
      return;
    spire->removeAllGeomObjects();
  }

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

void ViewSceneDialog::newGeometryValue()
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
    if (!spire)
    {
      LOG_DEBUG("Logical error: Spire lock not acquired.");
      return;
    }

    int port = 0;
    std::vector<std::string> objectNames;
    std::vector<std::string> validObjects;
    for (auto it = geomData->begin(); it != geomData->end(); ++it, ++port)
    {
      auto obj = *it;
      auto name = obj->uniqueID();
      auto displayName = QString::fromStdString(name).split('_').at(1);
      objectNames.push_back(displayName.toStdString());
      if (!isObjectUnselected(displayName.toStdString()))
      {
        auto realObj = boost::dynamic_pointer_cast<Graphics::Datatypes::GeometryObjectSpire>(obj);
        if (realObj)
        {
          spire->handleGeomObject(realObj, port);
          validObjects.push_back(name);
#ifdef BUILD_TESTING
          sendScreenshotDownstreamForTesting();
#endif
        }
      }
    }
    spire->gcInvalidObjects(validObjects);

    sort(objectNames.begin(), objectNames.end());
    if (previousObjectNames_ != objectNames)
    {
      itemValueChanged_ = true;
      previousObjectNames_ = objectNames;
    }
    if (itemValueChanged_ && mConfigurationDock)
    {
      mConfigurationDock->removeAllItems();
      for (auto it = objectNames.begin(); it != objectNames.end(); ++it)
      {
        std::string name = *it;
        if (isObjectUnselected(name))
        {
          mConfigurationDock->addItem(QString::fromStdString(name), false);
        }
        else
        {
          mConfigurationDock->addItem(QString::fromStdString(name), true);
        }
      }
      itemValueChanged_ = false;
    }
  }
  else
  {
    if (!spire)
      return;
    spire->removeAllGeomObjects();
  }

  if (saveScreenshotOnNewGeometry_)
  {
    screenshotClicked();
  }

  //TODO IMPORTANT: we need some call somewhere to clear the transient geometry list once spire/ES has received the list of objects. They take up lots of memory...
  //state_->setTransientValue(Parameters::GeomData, boost::shared_ptr<std::list<boost::shared_ptr<Core::Datatypes::GeometryObject>>>(), false);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::menuMouseControlChanged(int index)
{
  auto spire = mSpire.lock();
  if (!spire)
    return;

  if (index == 0)
  {
    spire->setMouseMode(SRInterface::MOUSE_OLDSCIRUN);
    Core::Preferences::Instance().useNewViewSceneMouseControls.setValue(false);
  }
  else
  {
    spire->setMouseMode(SRInterface::MOUSE_NEWSCIRUN);
    Core::Preferences::Instance().useNewViewSceneMouseControls.setValue(true);
  }
  mConfigurationDock->updateZoomOptionVisibility();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::autoViewClicked()
{
  auto spireLock = mSpire.lock();
  spireLock->doAutoView();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::showOrientationChecked(bool value)
{
  auto spire = mSpire.lock();
  spire->showOrientation(value);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::viewBarButtonClicked()
{
  hideViewBar_ = !hideViewBar_;
  mViewBar->setHidden(hideViewBar_);
  mDownViewBox->setCurrentIndex(0);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::viewAxisSelected(int index)
{
  mUpVectorBox->clear();
  mUpVectorBox->addItem("------");
  switch (index)
  {
  case 0: //default selection no value
    break;
  case 1: //Look down on +X Vector
    mUpVectorBox->addItem("+Y");
    mUpVectorBox->addItem("-Y");
    mUpVectorBox->addItem("+Z");
    mUpVectorBox->addItem("-Z");
    break;
  case 2: //Look down on +Y Vector
    mUpVectorBox->addItem("+X");
    mUpVectorBox->addItem("-X");
    mUpVectorBox->addItem("+Z");
    mUpVectorBox->addItem("-Z");
    break;
  case 3: //Look down on +Z Vector
    mUpVectorBox->addItem("+X");
    mUpVectorBox->addItem("-X");
    mUpVectorBox->addItem("+Y");
    mUpVectorBox->addItem("-Y");
    break;
  case 4: //Look down on -X Vector
    mUpVectorBox->addItem("+Y");
    mUpVectorBox->addItem("-Y");
    mUpVectorBox->addItem("+Z");
    mUpVectorBox->addItem("-Z");
    break;
  case 5: //Look down on -Y Vector
    mUpVectorBox->addItem("+X");
    mUpVectorBox->addItem("-X");
    mUpVectorBox->addItem("+Z");
    mUpVectorBox->addItem("-Z");
    break;
  case 6: //Look down on -Z Vector
    mUpVectorBox->addItem("+X");
    mUpVectorBox->addItem("-X");
    mUpVectorBox->addItem("+Y");
    mUpVectorBox->addItem("-Y");
    break;
  }

}

//------------------------------------------------------------------------------
void ViewSceneDialog::viewVectorSelected(int index)
{
  int downIndex = mDownViewBox->currentIndex();
  glm::vec3 up = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 view = glm::vec3(0.0f, 0.0f, 0.0f);
  switch (downIndex)
  {
  case 0:
    break;
  case 1:	//+X axis view
    view.x = 1.0f;
    lookDownAxisX(index, up);
    break;
  case 2:	//+Y axis view
    view.y = 1.0f;
    lookDownAxisY(index, up);
    break;
  case 3:	//+Z axis view
    view.z = 1.0f;
    lookDownAxisZ(index, up);
    break;
  case 4:	//-X axis view
    view.x = -1.0f;
    lookDownAxisX(index, up);
    break;
  case 5:	//-Y axis view
    view.y = 1.0f;
    lookDownAxisY(index, up);
    break;
  case 6:	//-Z axis view
    view.z = -1.0f;
    lookDownAxisZ(index, up);
    break;
  }
  if (index > 0)
  {
    std::shared_ptr<SRInterface> spire = mSpire.lock();
    spire->setView(view, up);
    viewBarButtonClicked();
  }
}

//------------------------------------------------------------------------------
void ViewSceneDialog::lookDownAxisX(int upIndex, glm::vec3& up)
{
  switch (upIndex)
  {
  case 0:
    break;
  case 1: //+Y axis
    up.y = 1.0f;
    break;
  case 2: //-Y axis
    up.y = -1.0f;
    break;
  case 3: //+Z axis
    up.z = 1.0f;
    break;
  case 4: //-Z axis
    up.z = -1.0f;
    break;
  }
}

void ViewSceneDialog::lookDownAxisY(int upIndex, glm::vec3& up)
{
  switch (upIndex)
  {
  case 0:
    break;
  case 1: //+X axis
    up.x = 1.0f;
    break;
  case 2: //-X axis
    up.x = -1.0f;
    break;
  case 3: //+Z axis
    up.z = 1.0f;
    break;
  case 4: //-Z axis
    up.z = -1.0f;
    break;
  }
}

void ViewSceneDialog::lookDownAxisZ(int upIndex, glm::vec3& up)
{
  switch (upIndex)
  {
  case 0:
    break;
  case 1: //+X axis
    up.x = 1.0f;
    break;
  case 2: //-X axis
    up.x = -1.0f;
    break;
  case 3: //+Y axis
    up.y = 1.0f;
    break;
  case 4: //-Y axis
    up.y = -1.0f;
    break;
  }
}

//------------------------------------------------------------------------------
void ViewSceneDialog::configurationButtonClicked()
{
  if (!mConfigurationDock)
  {
    addConfigurationDock(windowTitle());
    mConfigurationDock->setSampleColor(bgColor_);
    newGeometryValue();
  }

  showConfiguration_ = !mConfigurationDock->isVisible();
  mConfigurationDock->setEnabled(showConfiguration_);
  mConfigurationDock->setVisible(showConfiguration_);
}

//------------------------------------------------------------------------------
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
  }
}

//------------------------------------------------------------------------------
void ViewSceneDialog::setTransparencySortTypeContinuous(bool index)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setTransparencyRendertype(RenderState::TransparencySortType::CONTINUOUS_SORT);
  newGeometryValue();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::setTransparencySortTypeUpdate(bool index)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setTransparencyRendertype(RenderState::TransparencySortType::UPDATE_SORT);
  newGeometryValue();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::setTransparencySortTypeLists(bool index)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setTransparencyRendertype(RenderState::TransparencySortType::LISTS_SORT);
  newGeometryValue();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::handleUnselectedItem(const QString& name)
{
  itemValueChanged_ = true;
  unselectedObjectNames_.push_back(name.toStdString());
  newGeometryValue();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::handleSelectedItem(const QString& name)
{
  itemValueChanged_ = true;
  unselectedObjectNames_.erase(remove(unselectedObjectNames_.begin(), unselectedObjectNames_.end(), name.toStdString()), unselectedObjectNames_.end());
  newGeometryValue();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::selectAllClicked()
{
  itemValueChanged_ = true;
  unselectedObjectNames_.clear();
  newGeometryValue();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::deselectAllClicked()
{
  itemValueChanged_ = true;
  unselectedObjectNames_ = previousObjectNames_;
  newGeometryValue();
}

//------------------------------------------------------------------------------
void ViewSceneDialog::adjustZoomSpeed(int value)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setZoomSpeed(value);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::invertZoomClicked(bool value)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  spire->setZoomInverted(value);
  Core::Preferences::Instance().invertMouseZoom.setValue(value);
}

//------------------------------------------------------------------------------
bool ViewSceneDialog::isObjectUnselected(const std::string& name)
{
  return std::find(unselectedObjectNames_.begin(), unselectedObjectNames_.end(), name) != unselectedObjectNames_.end();
}

void ViewSceneDialog::addToolBar()
{
  mToolBar = new QToolBar(this);
  mToolBar->setStyleSheet("QToolBar { background-color: rgb(66,66,69); border: 1px solid black; color: black }");

  addConfigurationButton();
  addAutoViewButton();
  addScreenshotButton();
  //addObjectToggleMenu();

  glLayout->addWidget(mToolBar);

  addViewBar();
}

void ViewSceneDialog::addAutoViewButton()
{
  QPushButton* autoViewBtn = new QPushButton(this);
  autoViewBtn->setToolTip("Fit Object to Screen");
  autoViewBtn->setText("Auto View");
  autoViewBtn->setAutoDefault(false);
  autoViewBtn->setDefault(false);
  autoViewBtn->setShortcut(Qt::Key_0);
  connect(autoViewBtn, SIGNAL(clicked(bool)), this, SLOT(autoViewClicked()));
  mToolBar->addWidget(autoViewBtn);
  mToolBar->addSeparator();
}

void ViewSceneDialog::addScreenshotButton()
{
  QPushButton* screenshotButton = new QPushButton(this);
  screenshotButton->setToolTip("Take screenshot");
  screenshotButton->setText("Take screenshot");
  screenshotButton->setAutoDefault(false);
  screenshotButton->setDefault(false);
  screenshotButton->setShortcut(Qt::Key_F12);
  connect(screenshotButton, SIGNAL(clicked(bool)), this, SLOT(screenshotClicked()));
  mToolBar->addWidget(screenshotButton);

  mToolBar->addSeparator();
}

void ViewSceneDialog::addViewBarButton()
{
  QPushButton* viewBarBtn = new QPushButton();
  viewBarBtn->setToolTip("Show View Options");
  viewBarBtn->setText("Views");
  viewBarBtn->setAutoDefault(false);
  viewBarBtn->setDefault(false);
  connect(viewBarBtn, SIGNAL(clicked(bool)), this, SLOT(viewBarButtonClicked()));
  mToolBar->addWidget(viewBarBtn);
  mToolBar->addSeparator();
}

void ViewSceneDialog::addViewBar()
{
  mViewBar = new QToolBar(this);

  addViewOptions();
  hideViewBar_ = true;

  mViewBar->setHidden(hideViewBar_);

  glLayout->addWidget(mViewBar);

  addViewBarButton();
}

void ViewSceneDialog::addViewOptions()
{
  QLabel* axisLabel = new QLabel();
  axisLabel->setText("Look Down Axis: ");
  mViewBar->addWidget(axisLabel);

  mDownViewBox = new QComboBox();
  mDownViewBox->setToolTip("Vector pointing out of the screen");
  mDownViewBox->addItem("------");
  mDownViewBox->addItem("+X");
  mDownViewBox->addItem("+Y");
  mDownViewBox->addItem("+Z");
  mDownViewBox->addItem("-X");
  mDownViewBox->addItem("-Y");
  mDownViewBox->addItem("-Z");
  WidgetStyleMixin::toolbarStyle(mViewBar);
  connect(mDownViewBox, SIGNAL(currentIndexChanged(int)), this, SLOT(viewAxisSelected(int)));
  mViewBar->addWidget(mDownViewBox);
  mViewBar->addSeparator();

  QLabel* vectorLabel = new QLabel();
  vectorLabel->setText("Up Vector: ");
  mViewBar->addWidget(vectorLabel);

  mUpVectorBox = new QComboBox();
  mUpVectorBox->setToolTip("Vector pointing up");
  mUpVectorBox->addItem("------");
  connect(mUpVectorBox, SIGNAL(currentIndexChanged(int)), this, SLOT(viewVectorSelected(int)));
  mViewBar->addWidget(mUpVectorBox);
  mViewBar->addSeparator();
}

void ViewSceneDialog::addConfigurationButton()
{
  QPushButton* configurationButton = new QPushButton();
  configurationButton->setToolTip("Open/Close Configuration Menu");
  configurationButton->setText("Configure");
  configurationButton->setAutoDefault(false);
  configurationButton->setDefault(false);
  configurationButton->setShortcut(Qt::Key_F5);
  connect(configurationButton, SIGNAL(clicked(bool)), this, SLOT(configurationButtonClicked()));
  mToolBar->addWidget(configurationButton);
  mToolBar->addSeparator();
}

void ViewSceneDialog::addConfigurationDock(const QString& viewName)
{
  QString name = viewName + " Configuration";
  mConfigurationDock = new ViewSceneControlsDock(name, this);
  mConfigurationDock->setHidden(true);
  mConfigurationDock->setVisible(false);

  showConfiguration_ = false;
}

void ViewSceneDialog::hideConfigurationDock()
{
  if (mConfigurationDock)
  {
    showConfiguration_ = mConfigurationDock->isVisible();
    if (showConfiguration_)
    {
      configurationButtonClicked();
    }
  }
}

void ViewSceneDialog::showEvent(QShowEvent* evt)
{
  if (!shown_)
  {
    autoViewClicked();
    shown_ = true;
  }
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  ModuleDialogGeneric::showEvent(evt);
}

void ViewSceneDialog::hideEvent(QHideEvent* evt)
{
  hideConfigurationDock();
  ModuleDialogGeneric::hideEvent(evt);
}

void ViewSceneDialog::saveNewGeometryChanged(int state)
{
  saveScreenshotOnNewGeometry_ = state != 0;
}

void ViewSceneDialog::sendGeometryFeedbackToState(int x, int y)
{
  using namespace Core::Algorithms;
  Variable::List coords;
  coords.push_back(makeVariable("x", x));
  coords.push_back(makeVariable("y", y));
  state_->setValue(Parameters::GeometryFeedbackInfo, coords);
}

void ViewSceneDialog::takeScreenshot()
{
  if (!screenshotTaker_)
    screenshotTaker_ = new Screenshot(mGLWidget, this);

  screenshotTaker_->takeScreenshot();
}

void ViewSceneDialog::screenshotClicked()
{
  takeScreenshot();
  screenshotTaker_->saveScreenshot();
}

void ViewSceneDialog::sendScreenshotDownstreamForTesting()
{
  takeScreenshot();
  state_->setTransientValue(Parameters::ScreenshotData, screenshotTaker_->toMatrix(), false);
}
