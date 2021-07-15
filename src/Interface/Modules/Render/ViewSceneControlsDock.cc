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
using namespace SCIRun::Core::Algorithms;
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



  //WidgetStyleMixin::tabStyle(tabWidget);

  //tabWidget->setCurrentIndex(0);
}

static bool vsdPairComp(std::pair<ViewSceneDialog*, bool> a, std::pair<ViewSceneDialog*, bool> b)
{
  return std::get<0>(a)->getName() < std::get<0>(b)->getName();
}

void CameraLockControls::updateViewSceneTree()
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

void CameraLockControls::addGroup()
{
  ViewSceneDialog::viewSceneManager.addGroup();
  groupRemoveSpinBox_->setRange(0, ViewSceneDialog::viewSceneManager.getGroupCount() - 1);
}

void CameraLockControls::removeGroup()
{
  const uint32_t group = groupRemoveSpinBox_->value();
  ViewSceneDialog::viewSceneManager.removeGroup(group);
  groupRemoveSpinBox_->setRange(0, ViewSceneDialog::viewSceneManager.getGroupCount() - 1);
}

void CameraLockControls::viewSceneTreeClicked(QTreeWidgetItem* widgetItem, int column)
{
  auto p = widgetItem->parent();
  if (!p) return;
  uint32_t g = p->data(1, Qt::EditRole).toInt();
  const auto vs = widgetItem->data(1, Qt::EditRole).value<ViewSceneDialog*>();
  if (widgetItem->checkState(column) == Qt::Unchecked)
    ViewSceneDialog::viewSceneManager.removeViewSceneFromGroup(vs, g);
  else if (widgetItem->checkState(column) == Qt::Checked)
    ViewSceneDialog::viewSceneManager.moveViewSceneToGroup(vs, g);
}

void ColorOptions::setSampleColor(const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  currentBackgroundLabel_->setStyleSheet(styleSheet);
}

void LightControls::setLabelColor(QLabel* label, const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  label->setStyleSheet(styleSheet);
}

void FogControls::setFogColorLabel(const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  fogColorLabel_->setStyleSheet(styleSheet);
}

void MaterialsControls::setMaterialValues(double ambient, double diffuse, double specular, double shine, double)
{
  ambientDoubleSpinBox_->setValue(ambient);
  diffuseDoubleSpinBox_->setValue(diffuse);
  specularDoubleSpinBox_->setValue(specular);
  shininessDoubleSpinBox_->setValue(shine);
}

void FogControls::setFogValues(bool fogVisible, bool objectsOnly, bool useBGColor, double fogStart, double fogEnd)
{
  fogGroupBox_->setChecked(fogVisible);
  fogOnVisibleObjectsCheckBox_->setChecked(objectsOnly);
  fogUseBGColorCheckBox_->setChecked(useBGColor);
  fogStartDoubleSpinBox_->setValue(fogStart);
  fogEndDoubleSpinBox_->setValue(fogEnd);
}

void ScaleBarControls::setScaleBarValues(const ScaleBarData& scale)
{
  showScaleBarTextGroupBox_->setChecked(scale.visible);
  fontSizeSpinBox_->setValue(scale.fontSize);
  scaleBarLengthDoubleSpinBox_->setValue(scale.length);
  scaleBarHeightDoubleSpinBox_->setValue(scale.height);
  scaleBarMultiplierDoubleSpinBox_->setValue(scale.multiplier);
  numTicksSpinBox_->setValue(scale.numTicks);
  scaleBarUnitLineEdit_->setText(QString::fromStdString(scale.unit));
}

void InputControls::updateZoomOptionVisibility()
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

void ClippingPlaneControls::updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal)
{
  planeVisibleCheckBox_->setChecked(visible);
  showPlaneFrameCheckBox_->setChecked(showPlane);
  reversePlaneNormalCheckBox_->setChecked(reverseNormal);
}

void ClippingPlaneControls::updatePlaneControlDisplay(double x, double y, double z, double d)
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

  ScopedWidgetSignalBlocker xBlocker(xValueHorizontalSlider_);
  ScopedWidgetSignalBlocker yBlocker(yValueHorizontalSlider_);
  ScopedWidgetSignalBlocker zBlocker(zValueHorizontalSlider_);
  ScopedWidgetSignalBlocker dBlocker(dValueHorizontalSlider_);
  xValueHorizontalSlider_->setSliderPosition(x * 100);
  yValueHorizontalSlider_->setSliderPosition(y * 100);
  zValueHorizontalSlider_->setSliderPosition(z * 100);
  dValueHorizontalSlider_->setSliderPosition(d * 100);
}

// Set x and y sliders all the way right(100)
void OrientationAxesControls::setSliderDefaultPos()
{
  orientAxisXPos_->setValue(100);
  orientAxisYPos_->setValue(100);
}

// Set x and y sliders to half way(50)
void OrientationAxesControls::setSliderCenterPos()
{
  orientAxisXPos_->setValue(50);
  orientAxisYPos_->setValue(50);
}

VisibleItemManager::VisibleItemManager(QTreeWidget* itemList, ModuleStateHandle state)
  : itemList_(itemList), state_(state)
{
  connect(this, &VisibleItemManager::visibleItemChange, this, &VisibleItemManager::updateState);
  connect(this, &VisibleItemManager::meshComponentSelectionChange, this, &VisibleItemManager::updateState);
}

void VisibleItemManager::updateState()
{
  VariableList checkList;
  for (int i = 0; i < itemList_->topLevelItemCount(); ++i)
  {
    VariableList items;
    auto* item = itemList_->topLevelItem(i);
    items.emplace_back(makeVariable(item->text(0).toStdString(), item->checkState(0) == Qt::Checked));
    for (int j = 0; j < item->childCount(); ++j)
    {
      auto* child = item->child(j);
      items.emplace_back(makeVariable(child->text(0).toStdString(), child->checkState(0) == Qt::Checked));
    }
    checkList.push_back(makeVariable("graphicsItem", items));
  }
  state_->setValue(Core::Algorithms::Render::Parameters::VisibleItemListState, checkList);
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

void VisibleItemManager::initializeSavedStateMap()
{
  auto objs = state_->getValue(Core::Algorithms::Render::Parameters::VisibleItemListState).toVector();
  for (const auto& o : objs)
  {
    auto item = o.toVector();
    QString rootName;
    for (size_t i = 0; i < item.size(); ++i)
    {
      auto name = item[i].name().name();
      const auto checked = item[i].toBool();

      if (0 == i) // showfield
      {
        rootName = QString::fromStdString(name);
        topLevelItemMap_[rootName] = checked;
      }
      else
      {
        secondLevelItemMap_[rootName][QString::fromStdString(name)] = checked;
      }
    }
  }
}

void VisibleItemManager::addRenderItem(const QString& name)
{
  auto items = itemList_->findItems(name, Qt::MatchExactly);
  if (items.count() > 0)
  {
    return;
  }

  const QStringList names(name);
  auto item = new QTreeWidgetItem(itemList_, names);

  itemList_->addTopLevelItem(item);
  item->setCheckState(0, Qt::Checked);
  const auto topLevelItemStateIter = topLevelItemMap_.find(name);
  if (topLevelItemStateIter != topLevelItemMap_.end())
  {
    if (!topLevelItemStateIter->second)
      item->setCheckState(0, Qt::Unchecked);
  }
  if (name.contains("ShowField:"))
  {
    auto n = new QTreeWidgetItem(item, QStringList("Nodes"));
    auto e = new QTreeWidgetItem(item, QStringList("Edges"));
    auto f = new QTreeWidgetItem(item, QStringList("Faces"));

    const auto meshComponentCheckStates = secondLevelItemMap_.find(name);
    const auto hasSavedMeshFlags = meshComponentCheckStates != secondLevelItemMap_.end();
    if (hasSavedMeshFlags)
    {
      n->setCheckState(0, meshComponentCheckStates->second["Nodes"] ? Qt::Checked : Qt::Unchecked);
      e->setCheckState(0, meshComponentCheckStates->second["Edges"] ? Qt::Checked : Qt::Unchecked);
      f->setCheckState(0, meshComponentCheckStates->second["Faces"] ? Qt::Checked : Qt::Unchecked);
    }

  }
  if (name != "scale bar") //TODO: super hacky fix
    updateState();
}

void VisibleItemManager::updateCheckStates(const QString& name, const std::vector<bool>& checked)
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
    for (int j = 0; j < item->childCount(); ++j)
      item->child(j)->setCheckState(0, Qt::Checked);
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
    for (int j = 0; j < item->childCount(); ++j)
      item->child(j)->setCheckState(0, Qt::Unchecked);
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
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
  {
    auto& refToNonConstOption = const_cast<QStyleOptionViewItem&>(option);
    refToNonConstOption.showDecorationSelected = false;
    QStyledItemDelegate::paint(painter, refToNonConstOption, index);
  }
};

void ObjectSelectionControls::setupObjectListWidget()
{
  objectListWidget_->setItemDelegate(new FixMacCheckBoxes);
}

QColor LightControls::getLightColor() const
{
  return lightColor_;
}

void LightControls::selectLightColor()
{
  const auto title = lightNumber_ < 1 ? " Choose color for Headlight" : " Choose color for Light" + QString::number(lightNumber_);

  const auto newColor = QColorDialog::getColor(lightColor_, this, title);
  if (newColor.isValid())
  {
    lightColor_ = newColor;
    updateLightColor();
    setLabelColor(colorLabel_, lightColor_);
  }
}

AutoRotateControls::AutoRotateControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(rotateRightButton_, &QPushButton::clicked, parent, &ViewSceneDialog::autoRotateRight);
  connect(rotateLeftButton_, &QPushButton::clicked, parent, &ViewSceneDialog::autoRotateLeft);
  connect(rotateUpButton_, &QPushButton::clicked, parent, &ViewSceneDialog::autoRotateUp);
  connect(rotateDownButton_, &QPushButton::clicked, parent, &ViewSceneDialog::autoRotateDown);
  connect(autoRotateSpeedSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setAutoRotateSpeed(double)));
}

ColorOptions::ColorOptions(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(setBackgroundColorPushButton_, &QPushButton::clicked, parent, &ViewSceneDialog::assignBackgroundColor);
}

MaterialsControls::MaterialsControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(ambientDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setAmbientValue(double)));
  connect(diffuseDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setDiffuseValue(double)));
  connect(specularDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setSpecularValue(double)));
  connect(shininessDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setShininessValue(double)));
  //emissionDoubleSpinBox_->setDisabled(true);
}

FogControls::FogControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(fogStartDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setFogStartValue(double)));
  connect(fogEndDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setFogEndValue(double)));
  connect(fogGroupBox_, SIGNAL(clicked(bool)), parent, SLOT(setFogOn(bool)));
  fogOnVisibleObjectsCheckBox_->setDisabled(true);
  connect(fogUseBGColorCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(setFogUseBGColor(bool)));
  connect(fogColorPushButton_, SIGNAL(clicked()), parent, SLOT(assignFogColor()));
}

ObjectSelectionControls::ObjectSelectionControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  setupObjectListWidget();

  visibleItems_.reset(new VisibleItemManager(objectListWidget_, parent->state_));
  connect(selectAllPushButton_, &QPushButton::clicked, visibleItems_.get(), &VisibleItemManager::selectAllClicked);
  connect(deselectAllPushButton_, &QPushButton::clicked, visibleItems_.get(), &VisibleItemManager::deselectAllClicked);
  connect(objectListWidget_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), visibleItems_.get(), SLOT(updateVisible(QTreeWidgetItem*, int)));
  connect(visibleItems_.get(), SIGNAL(visibleItemChange()), parent, SIGNAL(newGeometryValueForwarder()));
  connect(visibleItems_.get(), SIGNAL(meshComponentSelectionChange(const QString&, const QString&, bool)),
    parent, SLOT(updateMeshComponentSelection(const QString&, const QString&, bool)));
}

OrientationAxesControls::OrientationAxesControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(orientationCheckableGroupBox_, SIGNAL(clicked(bool)), parent, SLOT(showOrientationChecked(bool)));
  connect(orientAxisSize_, SIGNAL(valueChanged(int)), parent, SLOT(setOrientAxisSize(int)));
  connect(orientAxisXPos_, SIGNAL(valueChanged(int)), parent, SLOT(setOrientAxisPosX(int)));
  connect(orientAxisYPos_, SIGNAL(valueChanged(int)), parent, SLOT(setOrientAxisPosY(int)));
  connect(orientDefaultPositionButton, &QPushButton::clicked, parent, &ViewSceneDialog::setDefaultOrientPos);
  connect(orientCenterPositionButton, &QPushButton::clicked, parent, &ViewSceneDialog::setCenterOrientPos);
  connect(orientDefaultPositionButton, &QPushButton::clicked, this, &OrientationAxesControls::setSliderDefaultPos);
  connect(orientCenterPositionButton, &QPushButton::clicked, this, &OrientationAxesControls::setSliderCenterPos);
}

ScaleBarControls::ScaleBarControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(showScaleBarTextGroupBox_, SIGNAL(clicked(bool)), parent, SLOT(setScaleBarVisible(bool)));
  connect(fontSizeSpinBox_, SIGNAL(valueChanged(int)), parent, SLOT(setScaleBarFontSize(int)));
  connect(scaleBarLengthDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarLength(double)));
  connect(scaleBarHeightDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarHeight(double)));
  connect(numTicksSpinBox_, SIGNAL(valueChanged(int)), parent, SLOT(setScaleBarNumTicks(int)));
  connect(scaleBarMultiplierDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarMultiplier(double)));
  connect(scaleBarUnitLineEdit_, SIGNAL(textEdited(const QString&)), parent, SLOT(setScaleBarUnitValue(const QString&)));
}

ClippingPlaneControls::ClippingPlaneControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

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
}

InputControls::InputControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  if (Preferences::Instance().useNewViewSceneMouseControls)
  {
    mouseControlComboBox_->setCurrentIndex(1);
  }
  else
  {
    mouseControlComboBox_->setCurrentIndex(0);
  }

  invertZoomCheckBox_->setChecked(Preferences::Instance().invertMouseZoom);

  updateZoomOptionVisibility();

  connect(saveScreenShotOnUpdateCheckBox_, SIGNAL(stateChanged(int)), parent, SLOT(saveNewGeometryChanged(int)));
  connect(mouseControlComboBox_, SIGNAL(currentIndexChanged(int)), parent, SLOT(menuMouseControlChanged(int)));
  connect(invertZoomCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(invertZoomClicked(bool)));
  connect(zoomSpeedHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(adjustZoomSpeed(int)));
}

CameraLockControls::CameraLockControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(addGroup_, SIGNAL(clicked()), this, SLOT(addGroup()));
  connect(removeGroup_, SIGNAL(clicked()), this, SLOT(removeGroup()));
  connect(viewSceneTreeWidget_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(viewSceneTreeClicked(QTreeWidgetItem*, int)));
  connect(&ViewSceneDialog::viewSceneManager, SIGNAL(groupsUpdatedSignal()), this, SLOT(updateViewSceneTree()));
  updateViewSceneTree();
  groupRemoveSpinBox_->setRange(0, 0);
}

DeveloperControls::DeveloperControls(ViewSceneDialog* parent) : QWidget(parent)
{
  setupUi(this);

  connect(toStringButton_, SIGNAL(clicked()), parent, SLOT(printToString()));
  connect(bugReportButton_, SIGNAL(clicked()), parent, SLOT(sendBugReport()));
}

LightControls::LightControls(ViewSceneDialog* viewScene, int lightNumber) : QWidget(viewScene), lightNumber_(lightNumber)
{
  setupUi(this);

  connect(lightCheckBox_, &QCheckBox::clicked,
    [this, viewScene](bool value) { viewScene->toggleLight(lightNumber_, value); });
  connect(lightAzimuthSlider_, &QSlider::valueChanged,
    [this, viewScene](int value) { viewScene->setLightAzimuth(lightNumber_, value); });
  connect(lightInclinationSlider_, &QSlider::valueChanged,
    [this, viewScene](int value) { viewScene->setLightInclination(lightNumber_, value); });
  connect(colorButton_, &QPushButton::clicked, this, &LightControls::selectLightColor);
  setLabelColor(colorLabel_, lightColor_ = Qt::white);

  connect(this, &LightControls::updateLightColor, [this, viewScene]() { viewScene->setLightColor(lightNumber_); });
}
