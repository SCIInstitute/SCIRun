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
#include <Core/Application/Application.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>
#include <Modules/Render/ViewScene.h>
#include <Interface/Modules/Render/Screenshot.h>
#include <Graphics/Glyphs/GlyphGeom.h>
#include <Graphics/Datatypes/GeometryImpl.h>
#include <Core/GeometryPrimitives/Transform.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Datatypes;
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
}

//------------------------------------------------------------------------------
ViewSceneDialog::ViewSceneDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent), mConfigurationDock(nullptr), shown_(false), itemValueChanged_(true),
  shiftdown_(false), selected_(false),
  clippingPlaneIndex_(0),screenshotTaker_(nullptr), saveScreenshotOnNewGeometry_(false),
  gid_(new DialogIdGenerator(name))
{
  counter_ = 1;
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));
  setFocusPolicy(Qt::StrongFocus);

  addToolBar();
  setupClippingPlanes(); 
  setupScaleBar();

  // Setup Qt OpenGL widget.
  QGLFormat fmt;
  fmt.setAlpha(true);
  fmt.setRgba(true);
  fmt.setDepth(true);
  fmt.setDoubleBuffer(true);
  fmt.setDepthBufferSize(24);

  mGLWidget = new GLWidget(new QtGLContext(fmt), parentWidget());
  connect(mGLWidget, SIGNAL(fatalError(const QString&)), this, SIGNAL(fatalError(const QString&)));
  connect(this, SIGNAL(mousePressSignalForTestingGeometryObjectFeedback(int, int)), this, SLOT(sendGeometryFeedbackToState(int, int)));

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
    if (!colorStr.empty())
    {
      ColorRGB color(colorStr);
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

  std::string filesystemRoot = Application::Instance().executablePath().string();
  std::string sep;
  sep += boost::filesystem::path::preferred_separator;
  Modules::Visualization::TextBuilder::setFSStrings(filesystemRoot, sep);
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
        auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
        if (realObj->uniqueID() == selName)
        {
          //selected_ = true;
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
    selected_ = false;
    restoreObjColor();
    newGeometryValue();
    //std::cout << "mousePressSignalForTestingGeometryObjectFeedback\n";
    Q_EMIT mousePressSignalForTestingGeometryObjectFeedback(event->x(), event->y());
  }
}

void ViewSceneDialog::mouseMoveEvent(QMouseEvent* event)
{
}

void ViewSceneDialog::wheelEvent(QWheelEvent* event)
{
  if (scaleBar_.visible)
  {
    updateScaleBarLength();
    scaleBarGeom_ = buildGeometryScaleBar();
    newGeometryValue();
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
    std::list<GeometryHandle> objList;

    for (auto it = geomData->begin(); it != geomData->end(); ++it)
    {
      auto obj = *it;
      auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
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

  int port = 0;
  std::vector<std::string> objectNames;
  std::vector<std::string> validObjects;
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
    //if (!spire)
    //{
    //  LOG_DEBUG("Logical error: Spire lock not acquired.");
    //  return;
    //}

    for (auto it = geomData->begin(); it != geomData->end(); ++it, ++port)
    {
      auto obj = *it;
      auto name = obj->uniqueID();
      auto displayName = QString::fromStdString(name).split('_').at(1);
      objectNames.push_back(displayName.toStdString());
      if (!isObjectUnselected(displayName.toStdString()))
      {
        auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(obj);
        if (realObj)
        {
          spire->handleGeomObject(realObj, port);
          validObjects.push_back(name);
        }
      }
    }
  }

  //add objects of its own
  //scale bar
  ++port;
  if (scaleBar_.visible && scaleBarGeom_)
  {
    auto name = scaleBarGeom_->uniqueID();
    auto displayName = QString::fromStdString(name).split('_').at(1);
    objectNames.push_back(name/*displayName.toStdString()*/);
      auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(scaleBarGeom_);
    if (realObj)
    {
      spire->handleGeomObject(realObj, port);
      validObjects.push_back(name);
    }
  }
  ++port;
  //clippingplanes
  for (auto i : clippingPlaneGeoms_)
  {
    auto name = i->uniqueID();
    auto displayName = QString::fromStdString(name).split('_').at(1);
    objectNames.push_back(name/*displayName.toStdString()*/);
      auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(i);
    if (realObj)
    {
      spire->handleGeomObject(realObj, port);
      validObjects.push_back(name);
    }
  }

  if (!validObjects.empty())
    spire->gcInvalidObjects(validObjects);

  if (!objectNames.empty())
  {
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
  //else
  //{
  //  if (!spire)
  //    return;
  //  spire->removeAllGeomObjects();
  //}

#ifdef BUILD_TESTING
  sendScreenshotDownstreamForTesting();
#endif

  if (saveScreenshotOnNewGeometry_)
  {
    screenshotClicked();
  }

  //TODO IMPORTANT: we need some call somewhere to clear the transient geometry list once spire/ES has received the list of objects. They take up lots of memory...
  //state_->setTransientValue(Parameters::GeomData, boost::shared_ptr<std::list<boost::shared_ptr<Core::Datatypes::GeometryObject>>>(), false);
}

void ViewSceneDialog::newOwnGeometryValue()
{
  LOG_DEBUG("ViewSceneDialog::asyncExecute before locking");

  Guard lock(Modules::Render::ViewScene::mutex_.get());

  LOG_DEBUG("ViewSceneDialog::asyncExecute after locking");

  auto spire = mSpire.lock();
  if (!spire)
    return;

  int port = 0;
  std::vector<std::string> objectNames;
  std::vector<std::string> validObjects;
  //add objects of its own
  //scale bar
  if (scaleBar_.visible && scaleBarGeom_)
  {
    auto name = scaleBarGeom_->uniqueID();
    auto displayName = QString::fromStdString(name).split('_').at(1);
    objectNames.push_back(displayName.toStdString());
    auto realObj = boost::dynamic_pointer_cast<GeometryObjectSpire>(scaleBarGeom_);
    if (realObj)
    {
      spire->handleGeomObject(realObj, port);
      validObjects.push_back(name);
    }
  }
  spire->gcInvalidObjects(validObjects);

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
    Preferences::Instance().useNewViewSceneMouseControls.setValue(false);
  }
  else
  {
    spire->setMouseMode(SRInterface::MOUSE_NEWSCIRUN);
    Preferences::Instance().useNewViewSceneMouseControls.setValue(true);
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
void ViewSceneDialog::autoViewOnLoadChecked(bool value)
{
  //TODO: Add to SRInterface
}

//------------------------------------------------------------------------------
void ViewSceneDialog::useOrthoViewChecked(bool value)
{
  //TODO: Add to SRInterface
}

//------------------------------------------------------------------------------
void ViewSceneDialog::showOrientationChecked(bool value)
{
  auto spire = mSpire.lock();
  spire->showOrientation(value);
}

//------------------------------------------------------------------------------
void ViewSceneDialog::showAxisChecked(bool value)
{
  //TODO: Add to SRInterface
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
    mConfigurationDock->setScaleBarValues(scaleBar_.visible, scaleBar_.fontSize, scaleBar_.length, scaleBar_.height,
      scaleBar_.multiplier, scaleBar_.numTicks, scaleBar_.visible, QString::fromStdString(scaleBar_.unit));
    setupMaterials();
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
    bool useBg = state_->getValue(Modules::Render::ViewScene::UseBGColor).toBool();
    if (useBg)
      setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
    else
      setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
    newGeometryValue();
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
  Preferences::Instance().invertMouseZoom.setValue(value);
}

//------------------------------------------------------------------------------
//--------------Clipping Plane Tools--------------------------------------------
void ViewSceneDialog::setClippingPlaneIndex(int index)
{
  int indexOffset = 7;
  clippingPlaneIndex_ = index + indexOffset;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneIndex(clippingPlaneIndex_);
  mConfigurationDock->updatePlaneSettingsDisplay(
    clippingPlanes_[clippingPlaneIndex_].visible,
    clippingPlanes_[clippingPlaneIndex_].showFrame,
    clippingPlanes_[clippingPlaneIndex_].reverseNormal);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneVisible(bool value)
{
  clippingPlanes_[clippingPlaneIndex_].visible = value;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneVisible(clippingPlanes_[clippingPlaneIndex_].visible);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneFrameOn(bool value)
{
  newGeometryValue();
  clippingPlanes_[clippingPlaneIndex_].showFrame = value;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneFrameOn(clippingPlanes_[clippingPlaneIndex_].showFrame);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::reverseClippingPlaneNormal(bool value)
{
  clippingPlanes_[clippingPlaneIndex_].reverseNormal = value;
  auto spire = mSpire.lock();
  if (spire)
    spire->reverseClippingPlaneNormal(clippingPlanes_[clippingPlaneIndex_].reverseNormal);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneX(int index)
{
  clippingPlanes_[clippingPlaneIndex_].x = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneX(clippingPlanes_[clippingPlaneIndex_].x);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneY(int index)
{
  clippingPlanes_[clippingPlaneIndex_].y = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneY(clippingPlanes_[clippingPlaneIndex_].y);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneZ(int index)
{
  clippingPlanes_[clippingPlaneIndex_].z = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneZ(clippingPlanes_[clippingPlaneIndex_].z);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::setClippingPlaneD(int index)
{
  clippingPlanes_[clippingPlaneIndex_].d = index / 100.0;
  auto spire = mSpire.lock();
  if (spire)
    spire->setClippingPlaneD(clippingPlanes_[clippingPlaneIndex_].d);
  updatClippingPlaneDisplay();
}

void ViewSceneDialog::updatClippingPlaneDisplay()
{
  mConfigurationDock->updatePlaneControlDisplay(
    clippingPlanes_[clippingPlaneIndex_].x,
    clippingPlanes_[clippingPlaneIndex_].y,
    clippingPlanes_[clippingPlaneIndex_].z,
    clippingPlanes_[clippingPlaneIndex_].d);

  //geometry
  buildGeomClippingPlanes();
  newGeometryValue();
}

//------------------------------------------------------------------------------
//-------------------Materials Bar Tools----------------------------------------
void ViewSceneDialog::setAmbientValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Ambient, value);
  setMaterialFactor(SRInterface::MAT_AMBIENT, value);
  newGeometryValue();
}

void ViewSceneDialog::setDiffuseValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Diffuse, value);
  setMaterialFactor(SRInterface::MAT_DIFFUSE, value);
  newGeometryValue();
}

void ViewSceneDialog::setSpecularValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Specular, value);
  setMaterialFactor(SRInterface::MAT_SPECULAR, value);
  newGeometryValue();
}

void ViewSceneDialog::setShininessValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Shine, value);
  setMaterialFactor(SRInterface::MAT_SHINE, value);
  newGeometryValue();
}

void ViewSceneDialog::setEmissionValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::Emission, value);
}

void ViewSceneDialog::setFogOn(bool value)
{
  state_->setValue(Modules::Render::ViewScene::FogOn, value);
  if (value)
    setFog(SRInterface::FOG_INTENSITY, 1.0);
  else
    setFog(SRInterface::FOG_INTENSITY, 0.0);
  newGeometryValue();
}

void ViewSceneDialog::setFogOnVisibleObjects(bool value)
{
  state_->setValue(Modules::Render::ViewScene::ObjectsOnly, value);
}

void ViewSceneDialog::setFogUseBGColor(bool value)
{
  state_->setValue(Modules::Render::ViewScene::UseBGColor, value);
  if (value)
    setFogColor(glm::vec4(bgColor_.red(), bgColor_.green(), bgColor_.blue(), 1.0));
  else
    setFogColor(glm::vec4(fogColor_.red(), fogColor_.green(), fogColor_.blue(), 1.0));
  newGeometryValue();
}

void ViewSceneDialog::setFogStartValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::FogStart, value);
  setFog(SRInterface::FOG_START, value);
  newGeometryValue();
}

void ViewSceneDialog::setFogEndValue(double value)
{
  state_->setValue(Modules::Render::ViewScene::FogEnd, value);
  setFog(SRInterface::FOG_END, value);
  newGeometryValue();
}

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
    newGeometryValue();
  }
}

//------------------------------------------------------------------------------
//-------------------Scale Bar Tools--------------------------------------------
void ViewSceneDialog::setScaleBarVisible(bool value)
{
  scaleBar_.visible = value;
  state_->setValue(Modules::Render::ViewScene::ShowScaleBar, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarFontSize(int value)
{
  scaleBar_.fontSize = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarFontSize, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarUnitValue(const QString& text)
{
  scaleBar_.unit = text.toStdString();
  state_->setValue(Modules::Render::ViewScene::ScaleBarUnitValue, text.toStdString());
  setScaleBar();
}

void ViewSceneDialog::setScaleBarLength(double value)
{
  scaleBar_.length = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarLength, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarHeight(double value)
{
  scaleBar_.height = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarHeight, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarMultiplier(double value)
{
  scaleBar_.multiplier = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarMultiplier, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarNumTicks(int value)
{
  scaleBar_.numTicks = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarNumTicks, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBarLineWidth(double value)
{
  scaleBar_.lineWidth = value;
  state_->setValue(Modules::Render::ViewScene::ScaleBarLineWidth, value);
  setScaleBar();
}

void ViewSceneDialog::setScaleBar()
{
  if (scaleBar_.visible)
  {
    updateScaleBarLength();
    scaleBarGeom_ = buildGeometryScaleBar();
    newGeometryValue();
  }
}

// update scale bar geometries
GeometryHandle ViewSceneDialog::buildGeometryScaleBar()
{
  const int    numTicks = scaleBar_.numTicks;
  const double mult = scaleBar_.multiplier;
  double length = scaleBar_.projLength;
  const double height = scaleBar_.height;
  glm::vec4 color(1.0);
  glm::vec4 shift(1.9, 0.1, 0.0, 0.0);

  //figure out text length first
  size_t text_size = size_t(scaleBar_.fontSize);
  if (!textBuilder_.isInit())
    textBuilder_.initFreeType("FreeSans.ttf", text_size);
  else if (!textBuilder_.isValid())
    textBuilder_.loadNewFace("FreeSans.ttf", text_size);
  //text
  std::stringstream ss;
  std::string oneline;
  ss << scaleBar_.length * scaleBar_.multiplier << " " << scaleBar_.unit;
  oneline = ss.str();
  double text_len = 0.0;
  if (textBuilder_.isInit() && textBuilder_.isValid())
    text_len = textBuilder_.getStringLen(oneline);
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

  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> iboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(vboSize));
  std::shared_ptr<CPM_VAR_BUFFER_NS::VarBuffer> vboBufferSPtr(
    new CPM_VAR_BUFFER_NS::VarBuffer(iboSize));

  CPM_VAR_BUFFER_NS::VarBuffer* iboBuffer = iboBufferSPtr.get();
  CPM_VAR_BUFFER_NS::VarBuffer* vboBuffer = vboBufferSPtr.get();

  for (auto a : indices) iboBuffer->write(a);

  for (size_t i = 0; i < points.size(); i++) {
    vboBuffer->write(static_cast<float>(points[i].x()));
    vboBuffer->write(static_cast<float>(points[i].y()));
    vboBuffer->write(static_cast<float>(points[i].z()));
  }

  ss.str("");
  ss << "scale_bar" << scaleBar_.fontSize << scaleBar_.length <<
    scaleBar_.height << scaleBar_.numTicks << scaleBar_.projLength;
  std::string uniqueNodeID = ss.str();
  std::string vboName = uniqueNodeID + "VBO";
  std::string iboName = uniqueNodeID + "IBO";
  std::string passName = uniqueNodeID + "Pass";

  // Construct VBO.
  std::string shader = "Shaders/HudUniform";
  std::vector<SpireVBO::AttributeData> attribs;
  attribs.push_back(SpireVBO::AttributeData("aPos", 3 * sizeof(float)));
  std::vector<SpireSubPass::Uniform> uniforms;
  uniforms.push_back(SpireSubPass::Uniform("uTrans", shift));
  uniforms.push_back(SpireSubPass::Uniform("uColor", color));
  SpireVBO geomVBO = SpireVBO(vboName, attribs, vboBufferSPtr,
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

  geom->mIBOs.push_back(geomIBO);
  geom->mVBOs.push_back(geomVBO);
  geom->mPasses.push_back(pass);

  //text
  if (textBuilder_.isInit() && textBuilder_.isValid())
  {
    if (textBuilder_.getFaceSize() != text_size)
      textBuilder_.setFaceSize(text_size);
    textBuilder_.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
    Vector shift(1.9, 0.1, 0.0);
    Vector trans(-text_len + 5, 0.0, 0.0);
    textBuilder_.printString(oneline, shift, trans, uniqueNodeID, geom);
  }

  return geom;
}

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
    //std::cout << "p1:\t" << p1.x << "\t" << p1.y << "\t" << p1.z << "\t" << p1.w << "\n";
    //std::cout << "p2:\t" << p2.x << "\t" << p2.y << "\t" << p2.z << "\t" << p2.w << "\n";
    //std::cout << "pp:\t" << pp.x << "\t" << pp.y << "\n";
  }
}

void ViewSceneDialog::buildGeomClippingPlanes()
{
  auto spire = mSpire.lock();
  if (!spire)
    return;
  StaticClippingPlanes* clippingPlanes = spire->getClippingPlanes();

  clippingPlaneGeoms_.clear();
  int index = 0;
  for (auto i : clippingPlanes->clippingPlanes)
  {
    if (clippingPlanes_[index].showFrame)
      buildGeometryClippingPlane(index, i, spire->getSceneBox());
    index++;
  }
}

//
void ViewSceneDialog::buildGeometryClippingPlane(int index, glm::vec4 plane, const BBox& bbox)
{
  Vector diag(bbox.diagonal());
  Point c(bbox.center());
  Vector n(plane.x, plane.y, plane.z);
  n.normalize();
  //Core::Geometry::Point p(c + (n * diag.length() / 2.0) * (plane.w));
  auto p(c + ((-plane.w) - Dot(c, n)) * n);
  //std::cout << "p0" << "\t" << p << "\n";
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
  //if (clippingPlanes_[index].reverseNormal)
  //  p = Core::Geometry::Point(n * plane.w);
  //else
  //  p = Core::Geometry::Point(-n * plane.w);
  //std::cout << "pp" << "\t" << p << "\n";
  auto p1 = p - axis1 * w / 2.0 - axis2 * h / 2.0;
  auto p2 = p + axis1 * w / 2.0 - axis2 * h / 2.0;
  auto p3 = p + axis1 * w / 2.0 + axis2 * h / 2.0;
  auto p4 = p - axis1 * w / 2.0 + axis2 * h / 2.0;

  std::stringstream ss;
  std::string uniqueNodeID;

  Graphics::GlyphGeom glyphs;
  glyphs.addClippingPlane(p1, p2, p3, p4, 0.01 * std::min(w, h),
    50, ColorRGB(), ColorRGB());
  ss << "clipping_plane" << index <<
    p1.x() << p1.y() << p1.z() <<
    p2.x() << p2.y() << p2.z() <<
    p3.x() << p3.y() << p3.z() <<
    p4.x() << p4.y() << p4.z();
  uniqueNodeID = ss.str();
  ColorScheme colorScheme(ColorScheme::COLOR_UNIFORM);
  RenderState renState;
  renState.set(RenderState::IS_ON, true);
  renState.set(RenderState::USE_TRANSPARENCY, false);
  renState.defaultColor = ColorRGB(0.4, 0.4, 1);
  renState.set(RenderState::USE_DEFAULT_COLOR, true);
  renState.set(RenderState::USE_NORMALS, true);
  renState.set(RenderState::IS_WIDGET, true);
  GeometryHandle geom(new GeometryObjectSpire(*gid_, uniqueNodeID, false));
  glyphs.buildObject(geom, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 1.0,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, bbox);

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
  GeometryHandle geom2(new GeometryObjectSpire(*gid_, ss.str(), false));
  glyphs2.buildObject(geom2, uniqueNodeID, renState.get(RenderState::USE_TRANSPARENCY), 0.2,
    colorScheme, renState, SpireIBO::PRIMITIVE::TRIANGLES, bbox);

  clippingPlaneGeoms_.push_back(geom);
  clippingPlaneGeoms_.push_back(geom2);
}

//set material
void ViewSceneDialog::setMaterialFactor(int factor, double value)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setMaterialFactor(static_cast<SRInterface::MatFactor>(factor), value);
}

//set fog
void ViewSceneDialog::setFog(int factor, double value)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setFog(static_cast<SRInterface::FogFactor>(factor), value);
}

void ViewSceneDialog::setFogColor(const glm::vec4 &color)
{
  auto spire = mSpire.lock();
  if (spire)
    spire->setFogColor(color/255.0);
}

//------------------------------------------------------------------------------
//-------------------Render Settings--------------------------------------------
void ViewSceneDialog::lightingChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::Lighting, value);
}

void ViewSceneDialog::showBBoxChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::ShowBBox, value);
}

void ViewSceneDialog::useClipChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::UseClip, value);
}

void ViewSceneDialog::stereoChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::Stereo, value);
}

void ViewSceneDialog::useBackCullChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::BackCull, value);
}

void ViewSceneDialog::displayListChecked(bool value)
{
  state_->setValue(Modules::Render::ViewScene::DisplayList, value);
}

void ViewSceneDialog::setStereoFusion(int value)
{
  double fusion = value / 100;
  state_->setValue(Modules::Render::ViewScene::StereoFusion, fusion);
}

void ViewSceneDialog::setPolygonOffset(int value)
{
  double offset = value / 100;
  state_->setValue(Modules::Render::ViewScene::PolygonOffset, offset);
}

void ViewSceneDialog::setTextOffset(int value)
{
  double offset = value / 100;
  state_->setValue(Modules::Render::ViewScene::TextOffset, offset);
}

void ViewSceneDialog::setFieldOfView(int value)
{
  state_->setValue(Modules::Render::ViewScene::FieldOfView, value);
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
    mConfigurationDock->setMaterialTabValues(0.2, 1.0, 0.4, 1.0, 1.0, false, true, true, 0.0, 0.71);
  }
  mConfigurationDock->setFogColorLabel(fogColor_);
}

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

namespace //TODO: move to appropriate location
{
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

void ViewSceneDialog::sendGeometryFeedbackToState(int x, int y)
{
  std::shared_ptr<SRInterface> spire = mSpire.lock();
  glm::mat4 trans = spire->getWidgetTransform().transform;

  ViewSceneFeedback vsf;
  vsf.transform = toSciTransform(trans);
  state_->setTransientValue(Parameters::GeometryFeedbackInfo, vsf);
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
  //wait for a couple frames to go by.
//  boost::this_thread::sleep(boost::posix_time::milliseconds(150));
  //std::cout << "sendScreenshotDownstreamForTesting " << std::endl;
  takeScreenshot();
  state_->setTransientValue(Parameters::ScreenshotData, screenshotTaker_->toMatrix(), false);
}
