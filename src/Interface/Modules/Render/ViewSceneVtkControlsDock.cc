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
#include <Interface/Modules/Render/ViewSceneVtkControlsDock.h>
#include <Interface/Modules/Render/ViewSceneVtkDialog.h>
#include <Interface/Modules/Render/ViewSceneVtkManager.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Modules/Visualization/ShowField.h>
#include <Modules/Visualization/ShowFieldVtk.h>
#include <Core/Logging/Log.h>
#include <Core/Utils/StringUtil.h>
#include <Interface/Modules/Base/CustomWidgets/CTK/ctkColorPickerButton.h>
#include <Interface/Modules/Base/CustomWidgets/CTK/ctkPopupWidget.h>

#include <qwt/qwt_knob.h>
#include <qwt/qwt_abstract_slider.h>

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

static bool vsdPairComp(std::pair<ViewSceneVtkDialog*, bool> a, std::pair<ViewSceneVtkDialog*, bool> b)
{
  return std::get<0>(a)->getName() < std::get<0>(b)->getName();
}

void CameraLockControlsVtk::updateViewSceneTree()
{
  viewSceneTreeWidget_->clear();

  int numGroups = ViewSceneVtkDialog::viewSceneManager.getGroupCount();
  std::vector<ViewSceneVtkDialog*> ungroupedMemebers;
  ViewSceneVtkDialog::viewSceneManager.getUngroupedViewScenesAsVector(ungroupedMemebers);

  for (int i = 0; i < numGroups; ++i)
  {
    auto group = new QTreeWidgetItem(viewSceneTreeWidget_, QStringList(QString::fromStdString("Group:" + std::to_string(i))));
    viewSceneTreeWidget_->addTopLevelItem(group);
    group->setData(1, Qt::EditRole, i);

    std::vector<ViewSceneVtkDialog*> groupMemebers;
    ViewSceneVtkDialog::viewSceneManager.getViewSceneGroupAsVector(i, groupMemebers);

    std::vector<std::pair<ViewSceneVtkDialog*, bool>> viewScenesToDisplay;
    for (auto vsd : groupMemebers)
      viewScenesToDisplay.emplace_back(vsd, true);
    for (auto vsd : ungroupedMemebers)
      viewScenesToDisplay.emplace_back(vsd, false);
    std::sort(viewScenesToDisplay.begin(), viewScenesToDisplay.end(), vsdPairComp);

    for (int j = 0; j < viewScenesToDisplay.size(); ++j)
    {
      auto item = new QTreeWidgetItem(group, QStringList(QString::fromStdString(std::get<0>(viewScenesToDisplay[j])->getName())));
      item->setCheckState(0, (std::get<1>(viewScenesToDisplay[j])) ? Qt::Checked : Qt::Unchecked);
      item->setData(1, Qt::EditRole, QVariant::fromValue(std::get<0>(viewScenesToDisplay[j])));
    }
  }

  viewSceneTreeWidget_->expandAll();
}

void CameraLockControlsVtk::addGroup()
{
  ViewSceneVtkDialog::viewSceneManager.addGroup();
  groupRemoveSpinBox_->setRange(0, ViewSceneVtkDialog::viewSceneManager.getGroupCount() - 1);
}

void CameraLockControlsVtk::removeGroup()
{
  const uint32_t group = groupRemoveSpinBox_->value();
  ViewSceneVtkDialog::viewSceneManager.removeGroup(group);
  groupRemoveSpinBox_->setRange(0, ViewSceneVtkDialog::viewSceneManager.getGroupCount() - 1);
}

void CameraLockControlsVtk::viewSceneTreeClicked(QTreeWidgetItem* widgetItem, int column)
{
  auto p = widgetItem->parent();
  if (!p) return;
  uint32_t g = p->data(1, Qt::EditRole).toInt();
  const auto vs = widgetItem->data(1, Qt::EditRole).value<ViewSceneVtkDialog*>();
  if (widgetItem->checkState(column) == Qt::Unchecked)
  {
    ViewSceneVtkDialog::viewSceneManager.removeViewSceneFromGroup(vs, g);
    vs->vsLog("");
  }
  else if (widgetItem->checkState(column) == Qt::Checked)
  {
    ViewSceneVtkDialog::viewSceneManager.moveViewSceneToGroup(vs, g);
    vs->vsLog(tr("In camera lock group %1").arg(g));
  }
}

void ColorOptionsVtk::setSampleColor(const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," + QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  currentBackgroundLabel_->setStyleSheet(styleSheet);
}

void MaterialsControlsVtk::setMaterialValues(double ambient, double diffuse, double specular, double shine, double)
{
  ambientDoubleSpinBox_->setValue(ambient);
  diffuseDoubleSpinBox_->setValue(diffuse);
  specularDoubleSpinBox_->setValue(specular);
  shininessDoubleSpinBox_->setValue(shine);
}

void FogControlsVtk::setFogValues(bool fogVisible, bool, bool useBGColor, double fogStart, double fogEnd)
{
  fogGroupBox_->setChecked(fogVisible);
  fogUseBGColorCheckBox_->setChecked(useBGColor);
  fogStartDoubleSpinBox_->setValue(fogStart);
  fogEndDoubleSpinBox_->setValue(fogEnd);
}

const QColor ScaleBarControlsVtk::buttonOutlineColor{"lightGray"};

void ScaleBarControlsVtk::setScaleBarValues(const ScaleBarDataVtk& scale)
{
  showScaleBarTextGroupBox_->setChecked(scale.visible);
  fontSizeSpinBox_->setValue(scale.fontSize);
  scaleBarLengthDoubleSpinBox_->setValue(scale.length);
  scaleBarHeightDoubleSpinBox_->setValue(scale.height);
  scaleBarMultiplierDoubleSpinBox_->setValue(scale.multiplier);
  numTicksSpinBox_->setValue(scale.numTicks);
  scaleBarUnitLineEdit_->setText(QString::fromStdString(scale.unit));
  grayLineColorDoubleSpinBox_->setValue(scale.lineColor);
  if (scale.visible) updateToolbarButton(ScaleBarControlsVtk::buttonOutlineColor);
}

void InputControlsVtk::updateZoomOptionVisibility()
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

void ClippingPlaneControlsVtk::updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal)
{
  planeVisibleCheckBox_->setChecked(visible);
  showPlaneFrameCheckBox_->setChecked(showPlane);
  reversePlaneNormalCheckBox_->setChecked(reverseNormal);
}

void ClippingPlaneControlsVtk::updatePlaneControlDisplay(double x, double y, double z, double d)
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
void OrientationAxesControlsVtk::setSliderDefaultPos()
{
  orientAxisXPos_->setValue(100);
  orientAxisYPos_->setValue(100);
}

// Set x and y sliders to half way(50)
void OrientationAxesControlsVtk::setSliderCenterPos()
{
  orientAxisXPos_->setValue(50);
  orientAxisYPos_->setValue(50);
}

VisibleItemManagerVtk::VisibleItemManagerVtk(QTreeWidget* itemList, ModuleStateHandle state) : itemList_(itemList), state_(state)
{
  connect(this, &VisibleItemManagerVtk::visibleItemChange, this, &VisibleItemManagerVtk::updateState);
  connect(this, &VisibleItemManagerVtk::meshComponentSelectionChange, this, &VisibleItemManagerVtk::updateState);
}

void VisibleItemManagerVtk::updateState()
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

bool VisibleItemManagerVtk::isVisible(const QString& name) const
{
  auto itemMatch = itemList_->findItems(name, Qt::MatchExactly);
  return itemMatch.size() == 1 && (itemMatch[0]->checkState(0) == Qt::Checked);
}

bool VisibleItemManagerVtk::containsItem(const QString& name) const
{
  auto itemMatch = itemList_->findItems(name, Qt::MatchExactly);
  return itemMatch.size() == 1;
}

std::vector<QString> VisibleItemManagerVtk::synchronize(const std::vector<GeometryBaseHandle>& geomList, const ShowFieldStatesMap& showFieldStates)
{
  std::vector<QString> displayNames;
  std::transform(geomList.begin(), geomList.end(), std::back_inserter(displayNames), [](const GeometryBaseHandle& geom) {
    auto parts = QString::fromStdString(geom->uniqueID()).split(GeometryObject::delimiter);
    return (parts.size() > 1) ? parts.at(1) : QString("scale bar");
  });

  for (int i = 0; i < itemList_->topLevelItemCount(); ++i)
  {
    if (std::find(displayNames.begin(), displayNames.end(), itemList_->topLevelItem(i)->text(0)) == displayNames.end()) delete itemList_->takeTopLevelItem(i);
  }

  for (const auto& name : displayNames)
  {
    if (!containsItem(name)) addRenderItem(name);

    auto stateIter = std::find_if(showFieldStates.begin(), showFieldStates.end(), [&name](const ShowFieldStatesMap::value_type& p) { return name.toStdString().find(p.first) != std::string::npos; });
    if (stateIter != showFieldStates.end())
    {
      auto state = stateIter->second;
      updateCheckStates(name, {state->getValue(Parameters::ShowNodes).toBool(), state->getValue(Parameters::ShowEdges).toBool(), state->getValue(Parameters::ShowFaces).toBool()});
    }
  }
  itemList_->sortItems(0, Qt::AscendingOrder);
  return displayNames;
}

void VisibleItemManagerVtk::initializeSavedStateMap()
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

      if (0 == i)  // showfield
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

void VisibleItemManagerVtk::addRenderItem(const QString& name)
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
    if (!topLevelItemStateIter->second) item->setCheckState(0, Qt::Unchecked);
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
  if (name != "scale bar")  // TODO: super hacky fix
    updateState();
}

void VisibleItemManagerVtk::updateCheckStates(const QString& name, const std::vector<bool>& checked)
{
  auto items = itemList_->findItems(name, Qt::MatchExactly);
  if (items.count() > 1)
  {
    return;
  }
  auto item = items[0];
  if (name.contains("ShowField:"))
  {
    auto nodes = item->child(2);  // TODO: brittle sort order
    auto edges = item->child(0);
    auto faces = item->child(1);
    std::vector<QTreeWidgetItem*> stuff{nodes, edges, faces};

    for (auto&& itemChecked : zip(stuff, checked))
    {
      QTreeWidgetItem* i;
      bool isChecked;
      boost::tie(i, isChecked) = itemChecked;
      i->setCheckState(0, isChecked ? Qt::Checked : Qt::Unchecked);
    }
  }
}

void VisibleItemManagerVtk::clear()
{
  if (itemList_->topLevelItemCount() > 0)
  {
    LOG_DEBUG("ViewScene items cleared");
    itemList_->clear();
  }
}

void VisibleItemManagerVtk::selectAllClicked()
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

void VisibleItemManagerVtk::deselectAllClicked()
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

void VisibleItemManagerVtk::updateVisible(QTreeWidgetItem* item, int column)
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
    if (!parent) Q_EMIT visibleItemChange();
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

void ObjectSelectionControlsVtk::setupObjectListWidget()
{
  objectListWidget_->setItemDelegate(new FixMacCheckBoxes);
}

AutoRotateControlsVtk::AutoRotateControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);

  connect(rotateRightButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::autoRotateRight);
  connect(rotateLeftButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::autoRotateLeft);
  connect(rotateUpButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::autoRotateUp);
  connect(rotateDownButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::autoRotateDown);
  connect(autoRotateSpeedSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setAutoRotateSpeed);
}

ColorOptionsVtk::ColorOptionsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);

  connect(setBackgroundColorPushButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::assignBackgroundColor);
}

MaterialsControlsVtk::MaterialsControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);

  connect(ambientDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setAmbientValue);
  connect(diffuseDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setDiffuseValue);
  connect(specularDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setSpecularValue);
  connect(shininessDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setShininessValue);
}

FogControlsVtk::FogControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton) : ViewSceneVtkControlPopupWidget(parent), LightButtonUpdaterVtk(toolbarButton, [this]() { toggleFog(); })
{
  setupUi(this);

  connect(fogStartDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setFogStartValue);
  connect(fogEndDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setFogEndValue);
  connect(fogGroupBox_, &QGroupBox::clicked, parent, &ViewSceneVtkDialog::setFogOn);
  connect(this, &FogControlsVtk::setFogTo, parent, &ViewSceneVtkDialog::setFogOn);
  connect(fogUseBGColorCheckBox_, &QCheckBox::clicked, parent, &ViewSceneVtkDialog::setFogUseBGColor);
  qobject_cast<QGridLayout*>(fogGroupBox_->layout())->addWidget(colorPickerButton_, 3, 0, 1, 2);
  linkedCheckable_ = [this]() { return fogGroupBox_->isChecked(); };
  connect(this, &FogControlsVtk::lightColorUpdated, parent, &ViewSceneVtkDialog::assignFogColor);
  connect(fogGroupBox_, &QGroupBox::toggled, [this]() { updateLightColor(); });
}

void FogControlsVtk::toggleFog()
{
  auto toggle = !fogGroupBox_->isChecked();
  fogGroupBox_->setChecked(toggle);
  Q_EMIT setFogTo(toggle);
}

ObjectSelectionControlsVtk::ObjectSelectionControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);

  setupObjectListWidget();

  visibleItems_.reset(new VisibleItemManagerVtk(objectListWidget_, parent->state_));
  connect(selectAllPushButton_, &QPushButton::clicked, visibleItems_.get(), &VisibleItemManagerVtk::selectAllClicked);
  connect(deselectAllPushButton_, &QPushButton::clicked, visibleItems_.get(), &VisibleItemManagerVtk::deselectAllClicked);
  connect(objectListWidget_, &QTreeWidget::itemClicked, visibleItems_.get(), &VisibleItemManagerVtk::updateVisible);
  connect(visibleItems_.get(), &VisibleItemManagerVtk::visibleItemChange, parent, &ViewSceneVtkDialog::newGeometryValueForwarder);
  connect(visibleItems_.get(), &VisibleItemManagerVtk::meshComponentSelectionChange, parent, &ViewSceneVtkDialog::updateMeshComponentSelection);
}

namespace {
template <typename Checkable>
void toggleCheckable(Checkable* box)
{
  box->setChecked(!box->isChecked());
}
}

OrientationAxesControlsVtk::OrientationAxesControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton)
    : ViewSceneVtkControlPopupWidget(parent), ButtonStylesheetTogglerVtk(toolbarButton, [this]() { toggleOrientation(); })
{
  setupUi(this);

  connect(orientationCheckableGroupBox_, &QGroupBox::toggled, [parent, this](bool b) {
    parent->showOrientationChecked(b);
    toggleButton();
  });
  connect(orientAxisSize_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setOrientAxisSize);
  connect(orientAxisXPos_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setOrientAxisPosX);
  connect(orientAxisYPos_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setOrientAxisPosY);
  connect(orientDefaultPositionButton, &QPushButton::clicked, parent, &ViewSceneVtkDialog::setDefaultOrientPos);
  connect(orientCenterPositionButton, &QPushButton::clicked, parent, &ViewSceneVtkDialog::setCenterOrientPos);
  connect(orientDefaultPositionButton, &QPushButton::clicked, this, &OrientationAxesControlsVtk::setSliderDefaultPos);
  connect(orientCenterPositionButton, &QPushButton::clicked, this, &OrientationAxesControlsVtk::setSliderCenterPos);
  linkedCheckable_ = [this]() { return orientationCheckableGroupBox_->isChecked(); };
}

void OrientationAxesControlsVtk::toggleOrientation()
{
  toggleCheckable(orientationCheckableGroupBox_);
}

void OrientationAxesControlsVtk::toggleButton()
{
  updateToolbarButton("lightGray");
}

ScreenshotControlsVtk::ScreenshotControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);
  connect(saveScreenShotOnUpdateCheckBox_, &QCheckBox::stateChanged, parent, &ViewSceneVtkDialog::saveNewGeometryChanged);
  connect(screenshotSaveAsButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::screenshotSaveAsClicked);
  connect(screenshotPathButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::setScreenshotDirectory);
}

ScaleBarControlsVtk::ScaleBarControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton)
    : ViewSceneVtkControlPopupWidget(parent), ButtonStylesheetTogglerVtk(toolbarButton, [this]() { toggleCheckable(showScaleBarTextGroupBox_); })
{
  setupUi(this);

  connect(showScaleBarTextGroupBox_, &QGroupBox::toggled, [parent, this](bool b) {
    parent->setScaleBarVisible(b);
    updateToolbarButton(buttonOutlineColor);
  });
  linkedCheckable_ = [this]() { return showScaleBarTextGroupBox_->isChecked(); };
  connect(fontSizeSpinBox_, qOverload<int>(&QSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setScaleBarFontSize);
  connect(scaleBarLengthDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setScaleBarLength);
  connect(scaleBarHeightDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setScaleBarHeight);
  connect(numTicksSpinBox_, qOverload<int>(&QSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setScaleBarNumTicks);
  connect(scaleBarMultiplierDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setScaleBarMultiplier);
  connect(grayLineColorDoubleSpinBox_, qOverload<double>(&QDoubleSpinBox::valueChanged), parent, &ViewSceneVtkDialog::setScaleBarLineColor);
  connect(scaleBarUnitLineEdit_, &QLineEdit::textEdited, parent, &ViewSceneVtkDialog::setScaleBarUnitValue);
}

ClippingPlaneControlsVtk::ClippingPlaneControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton)
    : ViewSceneVtkControlPopupWidget(parent), ButtonStylesheetTogglerVtk(toolbarButton, [this]() { toggleVisible(); })
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

  connect(planeButtonGroup_, BUTTON_GROUP_SIGNAL, parent, &ViewSceneVtkDialog::setClippingPlaneIndex);
  connect(planeVisibleCheckBox_, &QCheckBox::toggled, [parent, this](bool b) {
    parent->setClippingPlaneVisible(b);
    updateToolbarButton("lightGray");
  });
  linkedCheckable_ = [this]() { return planeVisibleCheckBox_->isChecked(); };
  connect(showPlaneFrameCheckBox_, &QCheckBox::clicked, parent, &ViewSceneVtkDialog::setClippingPlaneFrameOn);
  connect(reversePlaneNormalCheckBox_, &QCheckBox::clicked, parent, &ViewSceneVtkDialog::reverseClippingPlaneNormal);
  connect(xValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneX);
  connect(yValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneY);
  connect(zValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneZ);
  connect(dValueHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::setClippingPlaneD);

  // hide();
}

void ClippingPlaneControlsVtk::toggleVisible()
{
  toggleCheckable(planeVisibleCheckBox_);
}

InputControlsVtk::InputControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
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

  // connect(saveScreenShotOnUpdateCheckBox_, &QCheckBox::stateChanged, parent, &ViewSceneVtkDialog::saveNewGeometryChanged);
  connect(mouseControlComboBox_, qOverload<int>(&QComboBox::currentIndexChanged), parent, &ViewSceneVtkDialog::menuMouseControlChanged);
  connect(invertZoomCheckBox_, &QCheckBox::clicked, parent, &ViewSceneVtkDialog::invertZoomClicked);
  connect(zoomSpeedHorizontalSlider_, &QSlider::valueChanged, parent, &ViewSceneVtkDialog::adjustZoomSpeed);
}

CameraLockControlsVtk::CameraLockControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);

  connect(addGroup_, &QPushButton::clicked, this, &CameraLockControlsVtk::addGroup);
  connect(removeGroup_, &QPushButton::clicked, this, &CameraLockControlsVtk::removeGroup);
  connect(viewSceneTreeWidget_, &QTreeWidget::itemClicked, this, &CameraLockControlsVtk::viewSceneTreeClicked);
  connect(&ViewSceneVtkDialog::viewSceneManager, &ViewSceneVtkManager::groupsUpdatedSignal, this, &CameraLockControlsVtk::updateViewSceneTree);
  updateViewSceneTree();
  groupRemoveSpinBox_->setRange(0, 0);
}

DeveloperControlsVtk::DeveloperControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);

  connect(toStringButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::printToString);
  connect(bugReportButton_, &QPushButton::clicked, parent, &ViewSceneVtkDialog::sendBugReport);
}

ButtonStylesheetTogglerVtk::ButtonStylesheetTogglerVtk(QPushButton* toolbarButton, std::function<void()> whatToToggle) : toolbarButton_(toolbarButton), whatToToggle_(whatToToggle)
{
  QObject::connect(toolbarButton, &QPushButton::clicked, whatToToggle_);
}

LightButtonUpdaterVtk::LightButtonUpdaterVtk(QPushButton* toolbarButton, std::function<void()> whatToToggle) : ButtonStylesheetTogglerVtk(toolbarButton, whatToToggle)
{
  colorPickerButton_ = new ctkColorPickerButton("");
  QObject::connect(colorPickerButton_, &ctkColorPickerButton::colorChanged, [this]() { updateLightColor(); });
}

LightControlsVtk::LightControlsVtk(ViewSceneVtkDialog* viewScene, int lightNumber, QPushButton* toolbarButton)
    : ViewSceneVtkControlPopupWidget(viewScene), LightButtonUpdaterVtk(toolbarButton, [this]() { toggleLightOn(); }), lightNumber_(lightNumber)
{
  setupUi(this);
  auto lightLayout = qobject_cast<QGridLayout*>(layout());

#ifndef WIN32  // TODO: link error with Qwt--try upgrading
  lightAzimuthSlider_ = new QwtKnob(this);
  lightAzimuthSlider_->setTotalAngle(360);
  lightAzimuthSlider_->setScale(0, 360);
  lightAzimuthSlider_->setScaleStepSize(45);
  lightAzimuthSlider_->setMarkerStyle(QwtKnob::Triangle);
  lightAzimuthSlider_->setKnobWidth(65);
#else
  lightAzimuthSlider_ = new QSlider(Qt::Horizontal, this);
  lightAzimuthSlider_->setMinimum(0);
  lightAzimuthSlider_->setMaximum(360);
  lightAzimuthSlider_->setPageStep(45);
#endif
  lightAzimuthSlider_->setValue(180);
  lightAzimuthSlider_->setToolTip("Azimuth angle");
  lightLayout->addWidget(lightAzimuthSlider_, 1, 0);

  auto azLabel = new QLabel("Azimuth");
  azLabel->setAlignment(Qt::AlignCenter);
  lightLayout->addWidget(azLabel, 2, 0);
#ifndef WIN32
  lightInclinationSlider_ = new QwtKnob(this);
  lightInclinationSlider_->setTotalAngle(180);
  lightInclinationSlider_->setScale(0, 180);
  lightInclinationSlider_->setScaleStepSize(45);
  lightInclinationSlider_->setMarkerStyle(QwtKnob::Triangle);
  lightInclinationSlider_->setKnobWidth(65);
#else
  lightInclinationSlider_ = new QSlider(Qt::Horizontal, this);
  lightInclinationSlider_->setMinimum(0);
  lightInclinationSlider_->setMaximum(180);
  lightInclinationSlider_->setPageStep(45);
#endif
  lightInclinationSlider_->setValue(90);
  lightInclinationSlider_->setToolTip("Inclination angle");
  lightLayout->addWidget(lightInclinationSlider_, 1, 1);
  auto incLabel = new QLabel("Inclination");
  incLabel->setAlignment(Qt::AlignCenter);
  lightLayout->addWidget(incLabel, 2, 1);

  connect(lightAzimuthSlider_, &LightSliderTypeVtk::valueChanged, [this, viewScene](double value) { viewScene->setLightAzimuth(lightNumber_, value); });
  connect(lightInclinationSlider_, &LightSliderTypeVtk::valueChanged, [this, viewScene](double value) { viewScene->setLightInclination(lightNumber_, value); });

  linkedCheckable_ = [this]() { return lightCheckBox_->isChecked(); };
  lightLayout->addWidget(colorPickerButton_, 0, 1);
  connect(lightCheckBox_, &QCheckBox::toggled, [this, viewScene](bool value) { viewScene->toggleLight(lightNumber_, value); });
  connect(lightCheckBox_, &QCheckBox::toggled, [this]() { updateLightColor(); });

  colorPickerButton_->setColor(lightColor_ = Qt::white);

  connect(this, &LightControlsVtk::lightColorUpdated, [this, viewScene]() { viewScene->setLightColor(lightNumber_); });

  auto resetButton = new QPushButton("Reset angles");
  resetButton->setMaximumWidth(85);
  connect(resetButton, &QPushButton::clicked, this, &LightControlsVtk::resetAngles);
  lightLayout->addWidget(resetButton, 3, 0, 1, 2, Qt::AlignCenter);

#ifdef WIN32
  setMinimumHeight(120);
#endif
}

void LightControlsVtk::setLightOn(bool on)
{
  lightCheckBox_->setChecked(on);
}

void LightControlsVtk::toggleLightOn()
{
  lightCheckBox_->toggle();
}

bool LightControlsVtk::isLightOn() const
{
  return lightCheckBox_->isChecked();
}

void LightControlsVtk::resetAngles()
{
  if (lightAzimuthSlider_) lightAzimuthSlider_->setValue(180);
  if (lightInclinationSlider_) lightInclinationSlider_->setValue(90);
}

CompositeLightControlsVtk::CompositeLightControlsVtk(ViewSceneVtkDialog* parent, const std::vector<LightControlsVtk*>& secondaryLights) : ViewSceneVtkControlPopupWidget(parent), lights_(secondaryLights)
{
  tabs_ = new QTabWidget(this);
  WidgetStyleMixin::tabStyle(tabs_);
  int i = 0;
  for (auto& light : lights_)
  {
    tabs_->addTab(light, "Secondary light #" + QString::number(++i));
  }
  auto layout = new QHBoxLayout;
  setLayout(layout);
  layout->addWidget(tabs_);
  if (!lights_.empty()) setMinimumSize(lights_[0]->minimumSize());
}

QColor LightButtonUpdaterVtk::color() const
{
  return lightColor_;
}

void ButtonStylesheetTogglerVtk::updateToolbarButton(const QColor& color)
{
  if (!linkedCheckable_) return;
  if (linkedCheckable_())
  {
    const QColor complimentary(255 - color.red(), 255 - color.green(), 255 - color.blue());
    toolbarButton_->setStyleSheet("QPushButton { background-color: " + color.name() + "; color: " + complimentary.name() + " }");
  }
  else
  {
    toolbarButton_->setStyleSheet("");
  }
}

void LightButtonUpdaterVtk::updateLightColor()
{
  const auto newColor = colorPickerButton_->color();
  if (newColor.isValid())
  {
    lightColor_ = newColor;
    lightColorUpdated();
    updateToolbarButton(lightColor_);
  }
}

void LightButtonUpdaterVtk::setColor(const QColor& color)
{
  colorPickerButton_->setColor(color);
}

void LightControlsVtk::setAdditionalLightState(int azimuth, int inclination, bool on)
{
  if (lightAzimuthSlider_) lightAzimuthSlider_->setValue(azimuth);
  if (lightInclinationSlider_) lightInclinationSlider_->setValue(inclination);
  lightCheckBox_->setChecked(on);
  updateLightColor();
}

ViewAxisChooserControlsVtk::ViewAxisChooserControlsVtk(ViewSceneVtkDialog* parent) : ViewSceneVtkControlPopupWidget(parent)
{
  setupUi(this);

  connect(lookDownComboBox_, COMBO_BOX_ACTIVATED_STRING, this, &ViewAxisChooserControlsVtk::viewAxisSelected);
}

QString ViewAxisChooserControlsVtk::currentAxis() const
{
  return lookDownComboBox_->currentText();
}

void ViewAxisChooserControlsVtk::viewAxisSelected(const QString& name)
{
  upVectorComboBox_->clear();

  if (!name.contains("X"))
  {
    upVectorComboBox_->addItem("+X");
    upVectorComboBox_->addItem("-X");
  }
  if (!name.contains("Y"))
  {
    upVectorComboBox_->addItem("+Y");
    upVectorComboBox_->addItem("-Y");
  }
  if (!name.contains("Z"))
  {
    upVectorComboBox_->addItem("+Z");
    upVectorComboBox_->addItem("-Z");
  }
  upVectorComboBox_->setEnabled(true);
}

ViewSceneVtkControlPopupWidget::ViewSceneVtkControlPopupWidget(ViewSceneVtkDialog* parent) : QWidget(parent)
{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &ViewSceneVtkControlPopupWidget::showContextMenu);
	pinToggleAction_ = new QAction("Pin popup", this);
	pinToggleAction_->setCheckable(true);
	closeAction_ = new QAction("Close popup", this);
}

void ViewSceneVtkControlPopupWidget::showContextMenu(const QPoint& pos)
{
	QMenu contextMenu(tr("Context menu"), this);
	contextMenu.addAction(pinToggleAction_);
	contextMenu.addAction(closeAction_);
	contextMenu.exec(mapToGlobal(pos));
}

ViewSceneVtkToolBarController::ViewSceneVtkToolBarController(ViewSceneVtkDialog* dialog) : QObject(dialog), dialog_(dialog)
{
  SCIRun::Core::Preferences::Instance().toolBarPopupHideDelay.connectValueChanged([this](int) { updateDelays(); });
  SCIRun::Core::Preferences::Instance().toolBarPopupShowDelay.connectValueChanged([this](int) { updateDelays(); });
}

namespace {
struct PopupProperties
{
  Qt::Alignment alignment;
  Qt::Orientation orientation;
  ctkBasePopupWidget::VerticalDirection verticalDirection;
  Qt::LayoutDirection horizontalDirection;
};

constexpr PopupProperties bottomOutHorizontal{Qt::AlignBottom | Qt::AlignHCenter, Qt::Vertical, ctkBasePopupWidget::VerticalDirection::TopToBottom, Qt::LayoutDirectionAuto};
constexpr PopupProperties topOutHorizontal{Qt::AlignTop | Qt::AlignHCenter, Qt::Vertical, ctkBasePopupWidget::VerticalDirection::BottomToTop, Qt::LayoutDirectionAuto};
constexpr PopupProperties leftOutVertical{Qt::AlignLeft | Qt::AlignVCenter, Qt::Horizontal, ctkBasePopupWidget::VerticalDirection::TopToBottom, Qt::LayoutDirectionAuto};
constexpr PopupProperties rightOutVertical{Qt::AlignRight | Qt::AlignVCenter, Qt::Horizontal, ctkBasePopupWidget::VerticalDirection::TopToBottom, Qt::LeftToRight};

PopupProperties popupPropertiesFor(Qt::Orientation toolbarOrientation, Qt::ToolBarArea area, bool flipped)
{
  switch (toolbarOrientation)
  {
  case Qt::Horizontal:
  {
    switch (area)
    {
    case Qt::BottomToolBarArea: return flipped ? topOutHorizontal : bottomOutHorizontal;
    default: return flipped ? bottomOutHorizontal : topOutHorizontal;
    }
  }
  case Qt::Vertical:
  {
    switch (area)
    {
    case Qt::LeftToolBarArea: return flipped ? rightOutVertical : leftOutVertical;
    default: return flipped ? leftOutVertical : rightOutVertical;
    }
  }
  }
  return {};
}

QStyle::StandardPixmap oppositeArrow(const QPushButton* button)
{
  switch (static_cast<QStyle::StandardPixmap>(button->property(ViewSceneVtkToolBarController::DirectionProperty).toInt()))
  {
  case QStyle::SP_ArrowRight: return QStyle::SP_ArrowLeft;
  case QStyle::SP_ArrowLeft: return QStyle::SP_ArrowRight;
  case QStyle::SP_ArrowUp: return QStyle::SP_ArrowDown;
  case QStyle::SP_ArrowDown: return QStyle::SP_ArrowUp;
  default: return QStyle::SP_BrowserStop;
  }
}

QStyle::StandardPixmap outArrowForBarAt(Qt::ToolBarArea area)
{
  switch (area)
  {
  case Qt::LeftToolBarArea: return QStyle::SP_ArrowLeft;
  case Qt::RightToolBarArea: return QStyle::SP_ArrowRight;
  case Qt::BottomToolBarArea: return QStyle::SP_ArrowDown;
  case Qt::TopToolBarArea: return QStyle::SP_ArrowUp;
  default: return QStyle::SP_BrowserStop;
  }
}

}

void ViewSceneVtkToolBarController::setDefaultProperties(QToolBar* toolbar, ctkPopupWidget* popup)
{
  updatePopupProperties(toolbar, popup, false);

  popup->setShowDelay(SCIRun::Core::Preferences::Instance().toolBarPopupShowDelay);
  popup->setHideDelay(SCIRun::Core::Preferences::Instance().toolBarPopupHideDelay);
}

void ViewSceneVtkToolBarController::updateDelays()
{
  for (const auto& [toolbar, popups] : toolBarPopups_)
  {
    for (auto& popup : popups)
    {
      popup->setShowDelay(SCIRun::Core::Preferences::Instance().toolBarPopupShowDelay);
      popup->setHideDelay(SCIRun::Core::Preferences::Instance().toolBarPopupHideDelay);
    }
  }
}

void ViewSceneVtkToolBarController::registerPopup(QToolBar* toolbar, ctkPopupWidget* popup)
{
  connect(toolbar, &QToolBar::orientationChanged, [this, popup, toolbar](Qt::Orientation /*orientation*/) { updatePopupProperties(toolbar, popup, false); });
  connect(toolbar, &QToolBar::topLevelChanged, [this, popup, toolbar](bool /*topLevel*/) { updatePopupProperties(toolbar, popup, false); });
  connect(dialog_, &ViewSceneVtkDialog::fullScreenChanged, [this, popup, toolbar]() { updatePopupProperties(toolbar, popup, false); });
  toolBarPopups_[toolbar].push_back(popup);
}

void ViewSceneVtkToolBarController::updatePopupProperties(QToolBar* toolbar, ctkPopupWidget* popup, bool flipped)
{
  const auto props = popupPropertiesFor(toolbar->orientation(), dialog_->whereIs(toolbar), dialog_->isFullScreen() || flipped);
  popup->setAlignment(props.alignment);
  popup->setOrientation(props.orientation);
  popup->setVerticalDirection(props.verticalDirection);
  popup->setHorizontalDirection(props.horizontalDirection);
}

void ViewSceneVtkToolBarController::registerDirectionButton(QToolBar* toolbar, QPushButton* button)
{
  button->setProperty(FlipProperty, true);

  auto arrow = outArrowForBarAt(dialog_->whereIs(toolbar));
  button->setIcon(QApplication::style()->standardIcon(arrow));
  button->setProperty(DirectionProperty, static_cast<int>(arrow));

  connect(button, &QPushButton::clicked, [button, toolbar, this]() {
    const auto opp = oppositeArrow(button);
    button->setIcon(QApplication::style()->standardIcon(opp));
    button->setProperty(DirectionProperty, static_cast<int>(opp));
    bool flip = button->property(FlipProperty).toBool();
    for (auto& pop : toolBarPopups_[toolbar])
      updatePopupProperties(toolbar, pop, flip);
    button->setProperty(FlipProperty, !flip);
  });

  connect(toolbar, &QToolBar::topLevelChanged, [button, toolbar, this](bool /*topLevel*/) {
    button->setProperty(FlipProperty, true);
    auto outArrow = outArrowForBarAt(dialog_->whereIs(toolbar));
    button->setIcon(QApplication::style()->standardIcon(outArrow));
    button->setProperty(DirectionProperty, static_cast<int>(outArrow));
  });
}
