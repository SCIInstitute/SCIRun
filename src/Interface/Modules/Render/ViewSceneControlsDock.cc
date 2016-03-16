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

#include <Interface/Modules/Render/ViewScenePlatformCompatibility.h>
#include <Interface/Modules/Render/ViewSceneControlsDock.h>
#include <Core/Application/Preferences/Preferences.h>
#include <Core/Logging/Log.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Render;


ViewSceneControlsDock::ViewSceneControlsDock(const QString& name, ViewSceneDialog* parent) : QDockWidget(parent)
{
  setupUi(this);

  setHidden(true);
  setVisible(false);
  setEnabled(false);
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

  //-----------Objects Tab-----------------//
  connect(selectAllPushButton_, SIGNAL(clicked()), parent, SLOT(selectAllClicked()));
  connect(deselectAllPushButton_, SIGNAL(clicked()), parent, SLOT(deselectAllClicked()));
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
  connect(planeButtonGroup_, SIGNAL(buttonPressed(int)), parent, SLOT(setClippingPlaneIndex(int)));
  connect(planeVisibleCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(setClippingPlaneVisible(bool)));
  connect(showPlaneFrameCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(setClippingPlaneFrameOn(bool)));
  connect(reversePlaneNormalCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(reverseClippingPlaneNormal(bool)));
  connect(xValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneX(int)));
  connect(yValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneY(int)));
  connect(zValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneZ(int)));
  connect(dValueHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(setClippingPlaneD(int)));
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
  connect(showAxisCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(showAxisChecked(bool)));
  connect(showScaleBarTextGroupBox_, SIGNAL(clicked(bool)), parent, SLOT(setScaleBarVisible(bool)));
  connect(fontSizeSpinBox_, SIGNAL(valueChanged(int)), parent, SLOT(setScaleBarFontSize(int)));
  connect(scaleBarLengthDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarLength(double)));
  connect(scaleBarHeightDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarHeight(double)));
  connect(numTicksSpinBox_, SIGNAL(valueChanged(int)), parent, SLOT(setScaleBarNumTicks(int)));
  connect(scaleBarMultiplierDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarMultiplier(double)));
  connect(scaleBarLineWidthDoubleSpinBox_, SIGNAL(valueChanged(double)), parent, SLOT(setScaleBarLineWidth(double)));
  //-----------Controls Tab-------------------//
  connect(saveScreenShotOnUpdateCheckBox_, SIGNAL(stateChanged(int)), parent, SLOT(saveNewGeometryChanged(int)));
  connect(mouseControlComboBox_, SIGNAL(currentIndexChanged(int)), parent, SLOT(menuMouseControlChanged(int)));
  connect(contSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeContinuous(bool)));
  connect(updateSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeUpdate(bool)));
  connect(listSortRadioButton_, SIGNAL(clicked(bool)), parent, SLOT(setTransparencySortTypeLists(bool)));
  connect(invertZoomCheckBox_, SIGNAL(clicked(bool)), parent, SLOT(invertZoomClicked(bool)));
  connect(zoomSpeedHorizontalSlider_, SIGNAL(valueChanged(int)), parent, SLOT(adjustZoomSpeed(int)));

  connect(objectListWidget_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotChanged(QListWidgetItem*)));
  connect(this, SIGNAL(itemUnselected(const QString&)), parent, SLOT(handleUnselectedItem(const QString&)));
  connect(this, SIGNAL(itemSelected(const QString&)), parent, SLOT(handleSelectedItem(const QString&)));


  setSampleColor(Qt::black);

  WidgetStyleMixin::tabStyle(tabWidget);


  /////Set unused widgets to be not visible
  ////Clipping tab
  //tabWidget->removeTab(2);

  ///Object Tab
  tabWidget->setCurrentIndex(0);

  ///Render Tab
  shadeSettingsGroupBox_->setEnabled(false);
  globalSettingsGroupBox_->setEnabled(false);
  renderSliderFrame_->setEnabled(false);

  ///Materials Tab
  materialsFrame_->setEnabled(false);
  fogGroupBox_->setEnabled(false);

  ////View Tab
  autoRotateGroupBox_->setEnabled(false);
  viewOptionsGroupBox_->setEnabled(false);
  showScaleBarTextGroupBox_->setEnabled(false);

  ////Controls Tab
  transparencyGroupBox_->setVisible(false);

}

void ViewSceneControlsDock::setSampleColor(const QColor& color)
{
  QString styleSheet = "QLabel{ background: rgb(" + QString::number(color.red()) + "," +
    QString::number(color.green()) + "," + QString::number(color.blue()) + "); }";

  currentBackgroundLabel_->setStyleSheet(styleSheet);
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
  scaleBarLineWidthDoubleSpinBox_->setValue(lineWidth);
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

void ViewSceneControlsDock::addItem(const QString& name, bool checked)
{
  auto items = objectListWidget_->findItems(name, Qt::MatchExactly);
  if (items.count() > 0)
  {
    return;
  }

  QListWidgetItem* item = new QListWidgetItem(name, objectListWidget_);

  if (checked) 
    item->setCheckState(Qt::Checked);
  else 
    item->setCheckState(Qt::Unchecked);

  objectListWidget_->addItem(item);
}

void ViewSceneControlsDock::removeItem(const QString& name)
{
  auto items = objectListWidget_->findItems(name, Qt::MatchExactly);
  Q_FOREACH(QListWidgetItem* item, items)
  {
    objectListWidget_->removeItemWidget(item);
  }
}

void ViewSceneControlsDock::removeAllItems()
{
  if (objectListWidget_->count() > 1)
  {
    LOG_DEBUG("ViewScene items cleared" << std::endl);
    objectListWidget_->clear();
  }
}

void ViewSceneControlsDock::slotChanged(QListWidgetItem* item)
{
  if (item->checkState() == Qt::Unchecked)
  {
    LOG_DEBUG("Item " << item->text().toStdString() << " Unchecked!" << std::endl);
    Q_EMIT itemUnselected(item->text());
  }
  else if (item->checkState() == Qt::Checked)
  {
    LOG_DEBUG("Item " << item->text().toStdString() << " Checked!" << std::endl);
    Q_EMIT itemSelected(item->text());
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