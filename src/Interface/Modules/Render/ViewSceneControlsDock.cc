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


#include <Interface/Modules/Render/ViewScenePlatformCompatibility.h>
#include <Interface/Modules/Render/ViewSceneControlsDock.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Dataflow/Network/NullModuleState.h>
#include <Modules/Visualization/ShowField.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/StringUtil.h>

using namespace SCIRun;
using namespace SCIRun::Core;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Gui;
using namespace SCIRun::Render;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Modules::Render;
using namespace SCIRun::Modules::Visualization;

ViewSceneControlsDock::ViewSceneControlsDock(const QString& name, ViewSceneDialog* parent) : QDockWidget(parent)
{
  setupUi(this);

  setHidden(true);
  setVisible(false);
  setWindowTitle(name);
  setAllowedAreas(Qt::BottomDockWidgetArea);
  setFloating(true);
  setStyleSheet(parent->styleSheet());

  setupObjectListWidget();

  if (SCIRun::Core::Preferences::Instance().useNewViewSceneMouseControls)
  {
    mouseControlComboBox_->setCurrentIndex(1);
  }
  else
  {
    mouseControlComboBox_->setCurrentIndex(0);
  }

  invertZoomCheckBox_->setChecked(SCIRun::Core::Preferences::Instance().invertMouseZoom);

  updateZoomOptionVisibility();

  //----------- Developer Tab--------------//
  connect(toStringButton_, SIGNAL(clicked()), parent, SLOT(printToString()));
  connect(bugReportButton_, SIGNAL(clicked()), parent, SLOT(sendBugReport()));

  //-----------Objects Tab-----------------//
  visibleItems_.reset(new VisibleItemManager(objectListWidget_));
  connect(selectAllPushButton_, SIGNAL(clicked()), visibleItems_.get(), SLOT(selectAllClicked()));
  connect(deselectAllPushButton_, SIGNAL(clicked()), visibleItems_.get(), SLOT(deselectAllClicked()));
  connect(objectListWidget_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), visibleItems_.get(), SLOT(updateVisible(QTreeWidgetItem*, int)));
  connect(visibleItems_.get(), SIGNAL(visibleItemChange()), parent, SIGNAL(newGeometryValueForwarder()));
  connect(visibleItems_.get(), SIGNAL(meshComponentSelectionChange(const QString&, const QString&, bool)),
    parent, SLOT(updateMeshComponentSelection(const QString&, const QString&, bool)));

  connect(addGroup_, SIGNAL(clicked()), this, SLOT(addGroup()));
  connect(removeGroup_, SIGNAL(clicked()), this, SLOT(removeGroup()));
  connect(viewSceneTreeWidget_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(viewSceneTreeClicked(QTreeWidgetItem*, int)));
  connect(&ViewSceneDialog::viewSceneManager, SIGNAL(groupsUpdatedSignal()), this, SLOT(updateViewSceneTree()));
  updateViewSceneTree();
  groupRemoveSpinBox_->setRange(0, 0);

  //-----------Render Tab-----------------//
  connect(setBackgroundColorPushButton_, SIGNAL(clicked()), parent, SLOT(assignBackgroundColor()));
  connect(lightingCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(lightingChecked(bool)));
  connect(bboxCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(showBBoxChecked(bool)));
  connect(useClipCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(useClipChecked(bool)));
  connect(stereoCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(stereoChecked(bool)));
  connect(backCullCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(useBackCullChecked(bool)));
  connect(displayListCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(displayListChecked(bool)));
  connect(stereoFusionHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setStereoFusion(int)));
  connect(polygonOffsetHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setPolygonOffset(int)));
  connect(textOffsetHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setTextOffset(int)));
  connect(fieldOfViewHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setFieldOfView(int)));
  //-----------Clipping Tab-----------------//
  planeButtonGroup_->setId(plane1RadioButton_, 0);
  planeButtonGroup_->setId(plane2RadioButton_, 1);
  planeButtonGroup_->setId(plane3RadioButton_, 2);
  planeButtonGroup_->setId(plane4RadioButton_, 3);
  planeButtonGroup_->setId(plane5RadioButton_, 4);
  planeButtonGroup_->setId(plane6RadioButton_, 5);

  plane1RadioButton_->setStyleSheet("QRadioButton { color: rgb(219, 56, 22) }");
  plane2RadioButton_->setStyleSheet("QRadioButton { color: rgb(242, 102, 19) }");
  plane3RadioButton_->setStyleSheet("QRadioButton { color: rgb(205, 212, 74) }");
  plane4RadioButton_->setStyleSheet("QRadioButton { color: rgb(87, 184, 53) }");
  plane5RadioButton_->setStyleSheet("QRadioButton { color: rgb(126, 195, 237) }");
  plane6RadioButton_->setStyleSheet("QRadioButton { color: rgb(189, 54, 191) }");

  connect(planeButtonGroup_, SIGNAL(buttonPressed(int)), parent, SLOT(setClippingPlaneIndex(int)));
  connect(planeVisibleCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(setClippingPlaneVisible(bool)));
  connect(showPlaneFrameCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(setClippingPlaneFrameOn(bool)));
  connect(reversePlaneNormalCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(reverseClippingPlaneNormal(bool)));
  connect(xValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneX(int)));
  connect(yValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneY(int)));
  connect(zValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneZ(int)));
  connect(dValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneD(int)));
  //-----------Lights Tab-----------------//
  connect(headlightCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(toggleHeadLight(bool)));
  connect(headlightAzimuthSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setHeadLightAzimuth(int)));
  connect(headlightInclinationSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setHeadLightInclination(int)));
  connect(colorButton0, SIGNAL(clicked()), this, SLOT(selectLight0Color()));
  setLabelColor(color0, lightColors[0] = Qt::white);

  connect(light1CheckBox_, SIGNAL(clicked(bool)), parent, SLOT(toggleLight1(bool)));
  connect(light1AzimuthSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setLight1Azimuth(int)));
  connect(light1InclinationSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setLight1Inclination(int)));
  connect(colorButton1, SIGNAL(clicked()), this, SLOT(selectLight1Color()));
  setLabelColor(color1, lightColors[1] = Qt::white);

  connect(light2CheckBox_, SIGNAL(clicked(bool)), parent, SLOT(toggleLight2(bool)));
  connect(light2AzimuthSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setLight2Azimuth(int)));
  connect(light2InclinationSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setLight2Inclination(int)));
  connect(colorButton2, SIGNAL(clicked()), this, SLOT(selectLight2Color()));
  setLabelColor(color2, lightColors[2] = Qt::white);

  connect(light3CheckBox_, SIGNAL(clicked(bool)), parent, SLOT(toggleLight3(bool)));
  connect(light3AzimuthSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setLight3Azimuth(int)));
  connect(light3InclinationSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setLight3Inclination(int)));
  connect(colorButton3, SIGNAL(clicked()), this, SLOT(selectLight3Color()));
  setLabelColor(color3, lightColors[3] = Qt::white);

  connect(this, SIGNAL(updateLightColor(int)), parent, SLOT(setLightColor(int)));

  //-----------Materials Tab-----------------//
  connect(ambientDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setAmbientValue(double)));
  connect(diffuseDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setDiffuseValue(double)));
  connect(specularDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setSpecularValue(double)));
  connect(shininessDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setShininessValue(double)));
  connect(emissionDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setEmissionValue(double)));
  connect(fogStartDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setFogStartValue(double)));
  connect(fogEndDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setFogEndValue(double)));
  connect(fogGroupBox_, SIGNAL(clicked(bool)), parent, SLOT(setFogOn(bool)));
  connect(fogOnVisibleObjectsCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(setFogOnVisibleObjects(bool)));
  connect(fogUseBGColorCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(setFogUseBGColor(bool)));
  connect(fogColorPushButton_, SIGNAL(clicked()), parent, SLOT(assignFogColor()));
  //-----------View Tab-------------------//
  connect(autoViewOnLoadCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(autoViewOnLoadChecked(bool)));
  connect(orthoViewCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(useOrthoViewChecked(bool)));
  connect(orientationCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(showOrientationChecked(bool)));
  connect(orientAxisSize_, SIGNAL(valueChanged(int)), parent, SLOT(setOrientAxisSize(int)));
  connect(orientAxisXPos_, SIGNAL(valueChanged(int)), parent, SLOT(setOrientAxisPosX(int)));
  connect(orientAxisYPos_, SIGNAL(valueChanged(int)), parent, SLOT(setOrientAxisPosY(int)));
  connect(orientDefaultPositionButton, SIGNAL(clicked()), parent, SLOT(setDefaultOrientPos()));
  connect(orientCenterPositionButton, SIGNAL(clicked()), parent, SLOT(setCenterOrientPos()));
  connect(orientDefaultPositionButton, SIGNAL(clicked()), this, SLOT(setSliderDefaultPos()));
  connect(orientCenterPositionButton, SIGNAL(clicked()), this, SLOT(setSliderCenterPos()));
  connect(showScaleBarTextGroupBox_, SIGNAL(clicked(bool)), parent, SLOT(setScaleBarVisible(bool)));
  connect(fontSizeSpinBox_, SIGNAL(valueChanged(int)), parent, SLOT(setScaleBarFontSize(int)));
  connect(scaleBarLengthDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarLength(double)));
  connect(scaleBarHeightDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarHeight(double)));
  connect(numTicksSpinBox_, SIGNAL(valueChanged(int)), parent, SLOT(setScaleBarNumTicks(int)));
  connect(scaleBarMultiplierDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarMultiplier(double)));
  //connect(scaleBarLineWidthDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarLineWidth(double)));
  connect(scaleBarUnitLineEdit_, SIGNAL(textEdited(const QString&)), parent, SLOT(setScaleBarUnitValue(const QString&)));
  connect(rotateRightButton_, SIGNAL(clicked()), parent, SLOT(autoRotateRight()));
  connect(rotateLeftButton_, SIGNAL(clicked()), parent, SLOT(autoRotateLeft()));
  connect(rotateUpButton_, SIGNAL(clicked()), parent, SLOT(autoRotateUp()));
  connect(rotateDownButton_, SIGNAL(clicked()), parent, SLOT(autoRotateDown()));
  connect(autoRotateSpeedSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setAutoRotateSpeed(double)));

  //-----------Controls Tab-------------------//
  connect(saveScreenShotOnUpdateCheckBox_, SIGNAL(stateChanged(int)), parent, SLOT(saveNewGeometryChanged(int)));
  connect(mouseControlComboBox_, SIGNAL(currentIndexChanged(int)), parent, SLOT(menuMouseControlChanged(int)));
  connect(contSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeContinuous(bool)));
  connect(updateSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeUpdate(bool)));
  connect(listSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeLists(bool)));
  connect(invertZoomCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(invertZoomClicked(bool)));
  connect(zoomSpeedHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(adjustZoomSpeed(int)));

  setSampleColor(Qt::black);

  WidgetStyleMixin::tabStyle(tabWidget);

  /////Set unused widgets to be not visible
  ////Clipping tab
  //tabWidget->removeTab(2);

  ///Object Tab
  tabWidget->setCurrentIndex(0);

  ///Render Tab
  shadeSettingsGroupBox_->setEnabled(false);
  shadeSettingsGroupBox_->setVisible(false);
  globalSettingsGroupBox_->setEnabled(false);
  globalSettingsGroupBox_->setVisible(false);
  renderSliderFrame_->setEnabled(false);
  renderSliderFrame_->setVisible(false);

  ///Materials Tab
  //materialsFrame_->setEnabled(false);
  //fogGroupBox_->setEnabled(false);

  ////View Tab
  //autoRotateGroupBox_->setEnabled(false);
  //autoRotateGroupBox_->setVisible(false);
  viewOptionsGroupBox_->setEnabled(false);
  viewOptionsGroupBox_->setVisible(false);
  autoViewOnLoadCheckBox_->setVisible(false);
  orthoViewCheckBox_->setVisible(false);

  ////Controls Tab
  transparencyGroupBox_->setVisible(false);
}

static bool vsdPairComp(std::pair<ViewSceneDialog*, bool> a, std::pair<ViewSceneDialog*, bool> b)
{
  return std::get<0>(a)->getName() < std::get<0>(b)->getName();
}

void ViewSceneControlsDock::updateViewSceneTree()
{
  viewSceneTreeWidget_->clear();

  int numGroups = ViewSceneDialog::viewSceneManager.getGroupCount();
  std::vector<ViewSceneDialog*> ungroupedMemebers;
  ViewSceneDialog::viewSceneManager.getUngroupedViewScenesAsVector(ungroupedMemebers);

  for(int i = 0; i < numGroups; ++i)
  {
    auto group = new QTreeWidgetItem(viewSceneTreeWidget_, QStringList(QString::fromStdString("Group:" + std::to_string(i))));
    viewSceneTreeWidget_->addTopLevelItem(group);
    group->setData(1, Qt::EditRole, i);

    std::vector<ViewSceneDialog*> groupMemebers;
    ViewSceneDialog::viewSceneManager.getViewSceneGroupAsVector(i, groupMemebers);

    std::vector<std::pair<ViewSceneDialog*, bool>> viewScenesToDisplay;
    for(auto vsd : groupMemebers) viewScenesToDisplay.emplace_back(vsd, true);
    for(auto vsd : ungroupedMemebers) viewScenesToDisplay.emplace_back(vsd, false);
    std::sort(viewScenesToDisplay.begin(), viewScenesToDisplay.end(), vsdPairComp);

    for(int j = 0; j < viewScenesToDisplay.size(); ++j)
    {
      auto item = new QTreeWidgetItem(group, QStringList(QString::fromStdString(std::get<0>(viewScenesToDisplay[j])->getName())));
      item->setCheckState(0, (std::get<1>(viewScenesToDisplay[j])) ? Qt::Checked : Qt::Unchecked);
      item->setData(1, Qt::EditRole, QVariant::fromValue(std::get<0>(viewScenesToDisplay[j])));
    }
  }

  viewSceneTreeWidget_->expandAll();
}

void ViewSceneControlsDock::addGroup()
{
  ViewSceneDialog::viewSceneManager.addGroup();
  groupRemoveSpinBox_->setRange(0, ViewSceneDialog::viewSceneManager.getGroupCount() - 1);
}

void ViewSceneControlsDock::removeGroup()
{
  uint32_t group = groupRemoveSpinBox_->value();
  ViewSceneDialog::viewSceneManager.removeGroup(group);
  groupRemoveSpinBox_->setRange(0, ViewSceneDialog::viewSceneManager.getGroupCount() - 1);
}

void ViewSceneControlsDock::viewSceneTreeClicked(QTreeWidgetItem* widgetItem, int column)
{
  QTreeWidgetItem* p = widgetItem->parent();
  if(!p) return;
  uint32_t g = p->data(1, Qt::EditRole).toInt();
  ViewSceneDialog* vs = widgetItem->data(1, Qt::EditRole).value<ViewSceneDialog*>();
  if (widgetItem->checkState(column) == Qt::Unchecked)
    ViewSceneDialog::viewSceneManager.removeViewSceneFromGroup(vs, g);
  else if (widgetItem->checkState(column) == Qt::Checked)
    ViewSceneDialog::viewSceneManager.moveViewSceneToGroup(vs, g);
}

void ViewSceneControlsDock::setSampleColor(const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  currentBackgroundLabel_->setStyleSheet(styleSheet);
}

void ViewSceneControlsDock::setLabelColor(QLabel* label, const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  label->setStyleSheet(styleSheet);
}

void ViewSceneControlsDock::setFogColorLabel(const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  fogColorLabel_->setStyleSheet(styleSheet);
}

void ViewSceneControlsDock::setMaterialTabValues(double ambient, double diffuse, double specular, double shine, double emission,
  bool fogVisible, bool objectsOnly, bool useBGColor, double fogStart, double fogEnd)
{
  ambientDoubleSpinBox_->setValue(ambient);
  diffuseDoubleSpinBox_->setValue(diffuse);
  specularDoubleSpinBox_->setValue(specular);
  shininessDoubleSpinBox_->setValue(shine);
  emissionDoubleSpinBox_->setValue(emission);
  fogGroupBox_->setChecked(fogVisible);
  fogOnVisibleObjectsCheckBox_->setChecked(objectsOnly);
  fogUseBGColorCheckBox_->setChecked(useBGColor);
  fogStartDoubleSpinBox_->setValue(fogStart);
  fogEndDoubleSpinBox_->setValue(fogEnd);
}

void ViewSceneControlsDock::setScaleBarValues(bool visible, int fontSize, double length, double height, double multiplier,
  double numTicks, double lineWidth, const QString& unit)
{
  showScaleBarTextGroupBox_->setChecked(visible);
  fontSizeSpinBox_->setValue(fontSize);
  scaleBarLengthDoubleSpinBox_->setValue(length);
  scaleBarHeightDoubleSpinBox_->setValue(height);
  scaleBarMultiplierDoubleSpinBox_->setValue(multiplier);
  numTicksSpinBox_->setValue(numTicks);
  //scaleBarLineWidthDoubleSpinBox_->setValue(lineWidth);
  scaleBarUnitLineEdit_->setText(unit);
}

void ViewSceneControlsDock::setRenderTabValues(bool lighting, bool bbox, bool useClip, bool backCull, bool displayList,
  bool stereo, double stereoFusion, double polygonOffset, double textOffset, int fov)
{
  lightingCheckBox_->setChecked(lighting);
  bboxCheckBox_->setChecked(bbox);
  useClipCheckBox_->setChecked(useClip);
  backCullCheckBox_->setChecked(backCull);
  displayListCheckBox_->setChecked(displayList);
  stereoCheckBox_->setChecked(stereo);
  stereoFusionHorizontalSlider_->setSliderPosition(stereoFusion * 100);
  polygonOffsetHorizontalSlider_->setSliderPosition(polygonOffset * 100);
  textOffsetHorizontalSlider_->setSliderPosition(textOffset * 100);
  fieldOfViewHorizontalSlider_->setSliderPosition(fov);
}


void ViewSceneControlsDock::updateZoomOptionVisibility()
{
  if (SCIRun::Core::Preferences::Instance().useNewViewSceneMouseControls)
  {
    zoomBox_->setVisible(true);
    invertZoomCheckBox_->setVisible(true);
  }
  else
  {
    zoomBox_->setVisible(false);
    invertZoomCheckBox_->setVisible(false);
  }
}

void ViewSceneControlsDock::updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal)
{
  planeVisibleCheckBox_->setChecked(visible);
  showPlaneFrameCheckBox_->setChecked(showPlane);
  reversePlaneNormalCheckBox_->setChecked(reverseNormal);
}

void ViewSceneControlsDock::updatePlaneControlDisplay(double x, double y, double z, double d)
{
  QString xtext, ytext, ztext, dtext;
  if (x >= 0)
  {
    xtext = "  " + QString::number(x, 'f', 2);
  }
  else
  {
    xtext = QString::number(x, 'f', 2);
  }
  if (y >= 0)
  {
    ytext = "  " + QString::number(y, 'f', 2);
  }
  else
  {
    ytext = QString::number(y, 'f', 2);
  }
  if (z >= 0)
  {
    ztext = "  " + QString::number(z, 'f', 2);
  }
  else
  {
    ztext = QString::number(z, 'f', 2);
  }
  if (d >= 0)
  {
    dtext = "  " + QString::number(d, 'f', 2);
  }
  else
  {
    dtext = QString::number(d, 'f', 2);
  }

  xSliderValueLabel_->setText(xtext);
  ySliderValueLabel_->setText(ytext);
  zSliderValueLabel_->setText(ztext);
  dSliderValueLabel_->setText(dtext);

  xValueHorizontalSlider_->setSliderPosition(x * 100);
  yValueHorizontalSlider_->setSliderPosition(y * 100);
  zValueHorizontalSlider_->setSliderPosition(z * 100);
  dValueHorizontalSlider_->setSliderPosition(d * 100);
}

// Set x and y sliders all the way right(100)
void ViewSceneControlsDock::setSliderDefaultPos()
{
  orientAxisXPos_->setValue(100);
  orientAxisYPos_->setValue(100);
}

// Set x and y sliders to half way(50)
void ViewSceneControlsDock::setSliderCenterPos()
{
  orientAxisXPos_->setValue(50);
  orientAxisYPos_->setValue(50);
}

bool VisibleItemManager::isVisible(const QString& name) const
{
  auto itemMatch = itemList_->findItems(name, Qt::MatchExactly);
  return itemMatch.size() == 1 && (itemMatch[0]->checkState(0) == Qt::Checked);
}

bool VisibleItemManager::containsItem(const QString& name) const
{
  auto itemMatch = itemList_->findItems(name, Qt::MatchExactly);
  return itemMatch.size() == 1;
}

std::vector<QString> VisibleItemManager::synchronize(const std::vector<GeometryBaseHandle>& geomList,
  const ShowFieldStatesMap& showFieldStates)
{
  std::vector<QString> displayNames;
  std::transform(geomList.begin(), geomList.end(), std::back_inserter(displayNames),
    [](const GeometryBaseHandle& geom)
    {
      auto parts = QString::fromStdString(geom->uniqueID()).split(GeometryObject::delimiter);
      return (parts.size() > 1) ? parts.at(1) : QString("scale bar");
    }
  );
  for (int i = 0; i < itemList_->topLevelItemCount(); ++i)
  {
    if (std::find(displayNames.begin(), displayNames.end(), itemList_->topLevelItem(i)->text(0)) == displayNames.end())
      delete itemList_->takeTopLevelItem(i);
  }

  for (const auto& name : displayNames)
  {
    if (!containsItem(name))
      addRenderItem(name);

    auto stateIter = std::find_if(showFieldStates.begin(), showFieldStates.end(), [&name](const ShowFieldStatesMap::value_type& p) { return name.toStdString().find(p.first) != std::string::npos; });
    if (stateIter != showFieldStates.end())
    {
      auto state = stateIter->second;
      updateCheckStates(name, {
        state->getValue(Parameters::ShowNodes).toBool(),
        state->getValue(Parameters::ShowEdges).toBool(),
        state->getValue(Parameters::ShowFaces).toBool() });
    }
  }
  itemList_->sortItems(0, Qt::AscendingOrder);
  return displayNames;
}

void VisibleItemManager::addRenderItem(const QString& name)
{
  auto items = itemList_->findItems(name, Qt::MatchExactly);
  if (items.count() > 0)
  {
    return;
  }

  QStringList names(name);
  auto item = new QTreeWidgetItem(itemList_, names);

  itemList_->addTopLevelItem(item);
  item->setCheckState(0, Qt::Checked);
  if (name.contains("ShowField:"))
  {
    new QTreeWidgetItem(item, QStringList("Nodes"));
    new QTreeWidgetItem(item, QStringList("Edges"));
    new QTreeWidgetItem(item, QStringList("Faces"));
  }
}

void VisibleItemManager::updateCheckStates(const QString& name, std::vector<bool> checked)
{
  auto items = itemList_->findItems(name, Qt::MatchExactly);
  if (items.count() > 1)
  {
    return;
  }
  auto item = items[0];
  if (name.contains("ShowField:"))
  {
    auto nodes = item->child(2);  //TODO: brittle sort order
    auto edges = item->child(0);
    auto faces = item->child(1);
    std::vector<QTreeWidgetItem*> stuff{ nodes, edges, faces };

    for (auto&& itemChecked : zip(stuff, checked))
    {
      QTreeWidgetItem* i;
      bool isChecked;
      boost::tie(i, isChecked) = itemChecked;
      i->setCheckState(0, isChecked ? Qt::Checked : Qt::Unchecked);
    }
  }
}


void VisibleItemManager::clear()
{
  if (itemList_->topLevelItemCount() > 0)
  {
    LOG_DEBUG("ViewScene items cleared");
    itemList_->clear();
  }
}

void VisibleItemManager::selectAllClicked()
{
  itemList_->blockSignals(true);
  for (int i = 0; i < itemList_->topLevelItemCount(); ++i)
  {
    auto item = itemList_->topLevelItem(i);
    item->setCheckState(0, Qt::Checked);
    for (int i = 0; i < item->childCount(); ++i)
      item->child(i)->setCheckState(0, Qt::Checked);
  }
  itemList_->blockSignals(false);
  Q_EMIT visibleItemChange();
}

void VisibleItemManager::deselectAllClicked()
{
  itemList_->blockSignals(true);
  for (int i = 0; i < itemList_->topLevelItemCount(); ++i)
  {
    auto item = itemList_->topLevelItem(i);
    item->setCheckState(0, Qt::Unchecked);
    for (int i = 0; i < item->childCount(); ++i)
      item->child(i)->setCheckState(0, Qt::Unchecked);
  }
  itemList_->blockSignals(false);
  Q_EMIT visibleItemChange();
}

void VisibleItemManager::updateVisible(QTreeWidgetItem* item, int column)
{
  auto parent = item->parent();
  if (parent)
  {
    Q_EMIT meshComponentSelectionChange(parent->text(0), item->text(0), item->checkState(column) == Qt::Checked);
  }

  if (item->checkState(column) == Qt::Unchecked)
  {
    for (int i = 0; i < item->childCount(); ++i)
      item->child(i)->setCheckState(0, Qt::Unchecked);
    if (!parent)
      Q_EMIT visibleItemChange();
  }
  else if (item->checkState(column) == Qt::Checked)
  {
    for (int i = 0; i < item->childCount(); ++i)
      item->child(i)->setCheckState(0, Qt::Checked);

    if (parent)
      parent->setCheckState(0, Qt::Checked);
    else
      Q_EMIT visibleItemChange();
  }
}

class FixMacCheckBoxes : public QStyledItemDelegate
{
public:
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
  {
    QStyleOptionViewItem& refToNonConstOption = const_cast<QStyleOptionViewItem&>(option);
    refToNonConstOption.showDecorationSelected = false;
    QStyledItemDelegate::paint(painter, refToNonConstOption, index);
  }
};

void ViewSceneControlsDock::setupObjectListWidget()
{
  objectListWidget_->setItemDelegate(new FixMacCheckBoxes);
}

QColor ViewSceneControlsDock::getLightColor(int index) const
{
  return lightColors[index];
}

void ViewSceneControlsDock::selectLightColor(int index)
{
  QString title = index < 1 ? " Choose color for Headlight" : " Choose color for Light" + QString::number(index);

  auto newColor = QColorDialog::getColor(lightColors[index], this, title);
  if (newColor.isValid())
  {
    lightColors[index] = newColor;
    updateLightColor(index);
    switch (index)
    {
      case 0: setLabelColor(color0, newColor); break;
      case 1: setLabelColor(color1, newColor); break;
      case 2: setLabelColor(color2, newColor); break;
      case 3: setLabelColor(color3, newColor); break;
    }
  }
}
