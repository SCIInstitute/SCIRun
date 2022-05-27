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


#include <Interface/Modules/Visualization/CreateStandardColorMapDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/Visualization/CreateStandardColorMap.h>
#include <Core/Algorithms/Base/VariableHelper.h>

using namespace SCIRun;
using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Datatypes;

typedef SCIRun::Modules::Visualization::CreateStandardColorMap CreateStandardColorMapModule;

namespace
{
  // The preview has a fixed height, but the width is left dynamic so it matches the dialog width
  const double colormapPreviewHeight = 100;
}

#define DEVLOG 0

CreateStandardColorMapDialog::CreateStandardColorMapDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));

  addSpinBoxManager(resolutionSpin_, Parameters::ColorMapResolution);
  addDoubleSpinBoxManager(shiftSpin_, Parameters::ColorMapShift);
  addCheckBoxManager(invertCheck_, Parameters::ColorMapInvert);

  for (const auto& colorMapName : StandardColorMapFactory::getList())
  {
    colorMapNameComboBox_->addItem(QString::fromStdString(colorMapName));
  }

  colorMapNameComboBox_->addItem(QString::fromStdString("Custom"));

  connect(shiftSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CreateStandardColorMapDialog::setShiftSlider);
  connect(resolutionSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &CreateStandardColorMapDialog::setResolutionSlider);
  connect(shiftSpin_, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &CreateStandardColorMapDialog::updateColorMapPreview);
  connect(resolutionSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &CreateStandardColorMapDialog::updateColorMapPreview);

  connect(shiftSlider_, &QSlider::valueChanged, this, &CreateStandardColorMapDialog::setShiftSpinner);
  connect(resolutionSlider_, &QSlider::valueChanged, resolutionSpin_, &QSpinBox::setValue);
  connect(invertCheck_, &QCheckBox::toggled, this, &CreateStandardColorMapDialog::onInvertCheck);

  addRadioButtonGroupManager({predefinedColorMapRadioButton_, customColorMapRadioButton_}, Parameters::ColorMapOption);

  connect(customColorButton0_, &QPushButton::clicked, this, &CreateStandardColorMapDialog::selectCustomColorMin);
  connect(customColorButton1_, &QPushButton::clicked, this, &CreateStandardColorMapDialog::selectCustomColorMax);

  customColors_[0] = colorFromState(Parameters::CustomColor0);
  customColors_[1] = colorFromState(Parameters::CustomColor1);

  auto defaultMap = StandardColorMapFactory::create();
  auto rainbowIndex = colorMapNameComboBox_->findText("Rainbow", Qt::MatchExactly);
  if (rainbowIndex >= 0)
  {
    colorMapNameComboBox_->setCurrentIndex(rainbowIndex);
  }
  else
    qDebug() << "NO RAINBOW!";

  addComboBoxManager(colorMapNameComboBox_, Parameters::ColorMapName);
  connect(colorMapNameComboBox_, qOverload<int>(&QComboBox::currentIndexChanged), this, &CreateStandardColorMapDialog::updateColorMapPreview);
  connect(predefinedColorMapRadioButton_, &QPushButton::clicked, this, &CreateStandardColorMapDialog::updateColorMapPreview);
  connect(customColorMapRadioButton_, &QPushButton::clicked, this, &CreateStandardColorMapDialog::updateColorMapPreview);

  // Create preview window
  scene_ = new QGraphicsScene(this);
  previewColorMap_ = new ColormapPreview(scene_, state, pulling_, this);
  previewColorMap_->setFixedHeight(colormapPreviewHeight);
  qobject_cast<QVBoxLayout*>(groupBox->layout())->insertWidget(0, previewColorMap_);
  previewColorMap_->setStyleSheet(buildGradientString(*defaultMap));
  previewColorMap_->updateSize();
  previewColorMap_->show();
  connect(clearAlphaPointsToolButton_, &QPushButton::clicked, previewColorMap_, &ColormapPreview::clearAlphaPointGraphics);
}

void CreateStandardColorMapDialog::selectCustomColorMin()
{
  auto newColor = QColorDialog::getColor(customColors_[0], this, "Choose color");
  if (newColor.isValid()) customColors_[0] = newColor;

  colorToState(Parameters::CustomColor0, customColors_[0]);
  updateColorMapPreview();
}

void CreateStandardColorMapDialog::selectCustomColorMax()
{
  auto newColor = QColorDialog::getColor(customColors_[1], this, "Choose color");
  if (newColor.isValid()) customColors_[1] = newColor;

  colorToState(Parameters::CustomColor1, customColors_[1]);
  updateColorMapPreview();
}

void CreateStandardColorMapDialog::pullSpecial()
{
  previewColorMap_->updateFromState();
}

void CreateStandardColorMapDialog::resizeEvent(QResizeEvent *event)
{
  event->ignore();
  previewColorMap_->updateSize();
  pullSpecial();
  previewColorMap_->redraw();
}

// SCIRun changes the size when the window is shown, so that event is called
void CreateStandardColorMapDialog::showEvent(QShowEvent *event)
{
  event->ignore();
  resizeEvent(new QResizeEvent(size(), size()));
}

void ColormapPreview::updateFromState()
{
  auto points = getPointsFromState();

  if (points.empty())
  {
    removeCurrentLine();
    addDefaultLine();
  }
  else
  {
    if (alphaManager_.equals(points))
    {
      #if DEVLOG
      qDebug() << "ALPHA POINTS SAME, NOT CHANGING GUI FROM PULL";
      #endif
      return;
    }
    addPointsAndLineFromFile(points);
  }
  drawAlphaPolyline();
}

static ColorRGB toColorRGB(QColor& in)
{
  return ColorRGB(in.red() / 255.0f, in.green() / 255.0f, in.blue() / 255.0f);
}

void CreateStandardColorMapDialog::updateColorMapPreview()
{
  ColorMapHandle cmap;
  bool isCustom = customColorMapRadioButton_->isChecked();
  customColorButton0_->setEnabled(isCustom);
  customColorButton1_->setEnabled(isCustom);

  cmap = (isCustom) ?
    StandardColorMapFactory::create({toColorRGB(customColors_[0]), toColorRGB(customColors_[1])},
                                    "Custom", resolutionSlider_->value(),
                                    static_cast<double>(shiftSlider_->value()) / 100., invertCheck_->isChecked())
    :
    StandardColorMapFactory::create(colorMapNameComboBox_->currentText().toStdString(), resolutionSlider_->value(),
                                    static_cast<double>(shiftSlider_->value()) / 100., invertCheck_->isChecked());

  previewColorMap_->setStyleSheet(buildGradientString(*cmap));
}

QString CreateStandardColorMapDialog::buildGradientString(const ColorMap& cm) const
{
  //TODO: cache these values, GUI is slow to update.
  return QString::fromStdString(cm.styleSheet());
}

void CreateStandardColorMapDialog::setShiftSlider(double d)
{
  shiftSlider_->setValue(static_cast<int>(d * 100.));
}

void CreateStandardColorMapDialog::setResolutionSlider(int i)
{
  resolutionSlider_->setValue(i);
}

void CreateStandardColorMapDialog::setShiftSpinner(int i)
{
  shiftSpin_->setValue(static_cast<double>(i) / 100.);
}

void CreateStandardColorMapDialog::onInvertCheck(bool)
{
  updateColorMapPreview();
}

AlphaFunctionManager::AlphaFunctionManager()
{
}

ColormapPreview::ColormapPreview(QGraphicsScene* scene, ModuleStateHandle state,
  const boost::atomic<bool>& pulling, QWidget* parent)
  : QGraphicsView(scene, parent), alphaPath_(nullptr),
    alphaManager_(), dialogPulling_(pulling), state_(state)
{
}

void ColormapPreview::mousePressEvent(QMouseEvent* event)
{
  #if DEVLOG
  qDebug() << "\n\n~~~~~~~~~~~~~~~~~~pressed at" << event->pos();
  #endif

  QGraphicsView::mousePressEvent(event);
  auto pos = mapToScene(event->pos());

  // Points can either be deleted with shift+left click or a middle click. This also cancels adding a point
  if (event->buttons() & Qt::MiddleButton ||
      (event->buttons() & Qt::LeftButton && event->modifiers() == Qt::ShiftModifier))
  {
    if (selectedPoint_)
    {
      removePoint(selectedPoint_->center());
      selectedPoint_.reset();
    }
    else
      removePoint(pos);
  }
  // Add point
  else if (event->buttons() & Qt::LeftButton)
  {
    auto item = getItemAtPoint(pos);
    if (item)
    {
      pos = item->center(); // Adopt the item's pos to maintain its range and position
      removeItem(item);
      alphaManager_.erase(previewToAlphaPoint(pos));
    }

    // Make sure to get range before adding point, otherwise it will be counted as a current point
    auto range = alphaManager_.getPointRange(previewToAlphaPoint(pos));
    selectedPointRange_ = std::make_pair(range.first * clickableWidth(), range.second * clickableWidth());
    addPoint(pos, selectedPointRange_);
  }

  drawAlphaPolyline();
}

void ColormapPreview::mouseMoveEvent(QMouseEvent* event)
{
  QGraphicsView::mouseMoveEvent(event);

  if (event->buttons() & Qt::LeftButton && event->modifiers() != Qt::ShiftModifier)
  {
    auto pos = mapToScene(event->pos());

    // Clamp point to preview window
    if (pos.x() < 0) pos.setX(0);
    else if (pos.x() > clickableWidth())  pos.setX(clickableWidth());
    if (pos.y() < 0) pos.setY(0);
    else if (pos.y() > clickableHeight()) pos.setY(clickableHeight());

    // Move point by deleting and adding back
    if (selectedPoint_)
    {
      removePoint(selectedPoint_->center());
      selectedPoint_.reset();
      addPoint(pos, selectedPointRange_);
      drawAlphaPolyline();
    }
  }
}

void ColormapPreview::mouseReleaseEvent(QMouseEvent*)
{
  pushToState();
  selectedPoint_.reset();
}

static QPen alphaLinePen(Qt::red, 1);

// Draw line through middle of preview
void ColormapPreview::addDefaultLine()
{
  alphaPath_ = makeShared<QGraphicsPathItem>();
  QPainterPath path;

  auto alpha = state_->getValue(Parameters::DefaultAlphaValue).toDouble();
  auto pointHeight = alpha * clickableHeight();
  alphaPath_->setPen(alphaLinePen);
  path.moveTo(QPointF(0, pointHeight));
  path.lineTo(QPointF(clickableWidth(), pointHeight));
  alphaPath_->setPath(path);
  scene()->addItem(alphaPath_.get());
}

void ColormapPreview::updateSize()
{
  setSceneRect(QRect(0, 0, clickableWidth(), clickableHeight()));
  viewport()->update();
}

void ColormapPreview::removeCurrentLine()
{
  #if DEVLOG
  qDebug() << "alphaPath" << alphaPath_;
  #endif

  if (alphaPath_)
  {
    scene()->removeItem(alphaPath_.get());
    alphaPath_.reset();
  }
}

ColorMapPreviewPoint::ColorMapPreviewPoint(qreal x, qreal y)
  : QGraphicsEllipseItem(x - 4, y - 4, 8, 8), center_(x, y)
{
  setPen(QPen(Qt::white, 1));
  setBrush(QBrush(Qt::black));
  setZValue(1);
}

ColorMapPreviewPoint::ColorMapPreviewPoint(const QPointF& p)
  : ColorMapPreviewPoint(p.x(), p.y())
{}

void ColormapPreview::addPoint(const QPointF& point)
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif

  auto alphaPoint = previewToAlphaPoint(point);
  if (alphaManager_.alreadyExists(alphaPoint))
    return;

  auto item = new ColorMapPreviewPoint(point);
  scene()->addItem(item);
  previewPoints_.insert(item);
  selectedPoint_ = makeShared<ColorMapPreviewPoint>(point);
  alphaManager_.insert(alphaPoint);
}

// This adds a point but clamps it to a range for the x value
void ColormapPreview::addPoint(const QPointF& point, std::pair<double, double>& range)
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif

  // Clamp the x value between 2 nearest points, unless the edge of the scene is the limit
  auto clampedX = point.x();
  if (range.first >= 0 || range.first <= clickableWidth())
  {
    if (clampedX <= range.first) clampedX = range.first+1;
    else if (clampedX >= range.second) clampedX = range.second-1;
  }

  addPoint(QPointF(clampedX, point.y()));
}

void ColormapPreview::addPointsAndLineFromFile(const LogicalAlphaPointSet& pointsToLoad)
{
  removeCurrentLine();

  if (pointsToLoad.size() <= 0)
    addDefaultLine();
  else
    for (const auto& p : pointsToLoad)
      addPoint(alphaToPreviewPoint(p));
}

ColorMapPreviewPoint* ColormapPreview::getItemAtPoint(const QPointF& point) const
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif

  auto items = scene()->items(point);

  #if DEVLOG
  qDebug() << items;
  #endif

  ColorMapPreviewPoint* selectedItem = nullptr;
  for (auto item : items)
    if (auto c = dynamic_cast<ColorMapPreviewPoint*>(item))
    {
      selectedItem = c;
      break;
    }
  return selectedItem;
}

void ColormapPreview::removePoint(const QPointF& point)
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif

  #if DEVLOG
  qDebug() << "need to remove at" << point;
  #endif

  auto item = getItemAtPoint(point);
  if (item)
    removeItem(item);
}

void ColormapPreview::removeItem(ColorMapPreviewPoint* item)
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif

  scene()->removeItem(item);
  previewPoints_.erase(item);
  alphaManager_.erase(previewToAlphaPoint(item->center()));
  delete item;
}

  // This flips the y axis, because the Qt origin is the top left, but the alpha manager's origin is bottom left
QPointF& ColormapPreview::flipYAxis(QPointF& pos) const
{
  pos.setY(1.0 - pos.y());
  return pos;
}

// Converts from the preview window's space to the alpha manager's space
QPointF ColormapPreview::previewToAlphaPoint(QPointF pos) const
{
  return flipYAxis(normalizePoint(pos));
}

// Converts from the alpha manager's space to the preview window's space
QPointF ColormapPreview::alphaToPreviewPoint(QPointF pos) const
{
  return scalePointToWindow(flipYAxis(pos));
}

// Scale normalized point back to the preview's coordinate system
QPointF& ColormapPreview::scalePointToWindow(QPointF& pos) const
{
  pos.setX(pos.x() * clickableWidth());
  pos.setY(pos.y() * clickableHeight());
  return pos;
}

// This maps the preview coordinates to a normalized space for the alpha manager, which is size independent
QPointF& ColormapPreview::normalizePoint(QPointF& pos) const
{
  pos.setX(pos.x() / clickableWidth());
  pos.setY(pos.y() / clickableHeight());
  return pos;
}

bool AlphaFunctionManager::alreadyExists(const QPointF& point) const
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  printSet();
  #endif

  bool ret = false;
  if (alphaPoints_.count(point) > 0)
    ret = true;

  if (ret)
  {
    #if DEVLOG
    qDebug() << "\t\treturning true";
    #endif
    return ret;
  }
  const double x = point.x();
  ret = std::find_if(alphaPoints_.begin(), alphaPoints_.end(), [=](const QPointF& p) { return p.x() == x; }) != alphaPoints_.end();
  #if DEVLOG
  qDebug() << "\t\treturning" << ret;
  #endif
  return ret;
}

bool AlphaFunctionManager::empty() const
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif

  return size() <= 0;
}


void AlphaFunctionManager::insert(const QPointF& p)
{
  #if DEVLOG
  qDebug() << "inserting" << p;
  printSet();
  #endif

  alphaPoints_.insert(p);
}

// Returns a range of x values so the point can't go over the range
// Adding points cannot try to select a range where the nearby points are adjacent
std::pair<double, double> AlphaFunctionManager::getPointRange(const QPointF& p)
{
  // Default values. These need to be larger than the range [0,1] so we know if items are at the border
  double minX = -0.1;
  double maxX = 1.1;

  // Iterate until you find the nearest points
  auto it = alphaPoints_.begin();
  while (it != alphaPoints_.end())
  {
    auto x = it->x();
    if (x > p.x())
    {
      maxX = x; // When max is found, min is the previous value
      break;
    }
    else
      minX = x; // Update min to last value

    ++it;
  }
  return std::make_pair(minX, maxX);
}

void AlphaFunctionManager::printSet() const
{
  std::for_each(alphaPoints_.begin(), alphaPoints_.end(), [](const QPointF& p) { qDebug() << '\t' << p; });
}

void AlphaFunctionManager::erase(const QPointF& p)
{
  #if DEVLOG
  qDebug() << "erasing" << p;
  printSet();
  #endif

  alphaPoints_.erase(p);
}

size_t AlphaFunctionManager::size() const
{
  return alphaPoints_.size();
}

void AlphaFunctionManager::clear()
{
  alphaPoints_.clear();
}

void ColormapPreview::pushToState()
{
  if (dialogPulling_) return;
  Variable::List list;
  if (!alphaManager_.empty())
    list = getPointsAsVariableList();
  state_->setValue(Parameters::AlphaUserPointsVector, list);
}

double ColormapPreview::convertToAlphaValue(double v)
{
  return 1.0 - v;
}

LogicalAlphaPointSet ColormapPreview::getPointsFromState()
{
  auto statePoints = state_->getValue(Parameters::AlphaUserPointsVector).toVector();
  auto toQPointF = [](const Variable& v) { auto v2 = v.toVector(); return QPointF(v2[0].toDouble(), v2[1].toDouble()); };
  auto pairVec = toTypedVector<QPointF>(statePoints, toQPointF);
  return LogicalAlphaPointSet(pairVec.begin(), pairVec.end());
}

Variable::List ColormapPreview::getPointsAsVariableList()
{
  Variable::List alphaPointsVec;
  auto begin = alphaManager_.begin(), end = alphaManager_.end();
  std::for_each(begin, end, [&](const QPointF& p) {
    alphaPointsVec.emplace_back(Name("alphaPoint"),
    makeAnonymousVariableList(p.x(), p.y())); });
  return alphaPointsVec;
}

void ColormapPreview::drawAlphaPolyline()
{
  removeCurrentLine();
  if (alphaManager_.size() == 0)
  {
    addDefaultLine();
    return;
  }
  alphaPath_ = makeShared<QGraphicsPathItem>();
  alphaPath_->setPen(alphaLinePen);

  // Create path object and set from and to to the outer most point's y values
  QPainterPath path;
  auto start = alphaManager_.begin();
  auto end = alphaManager_.end(); std::advance(end, -1);
  QPointF from = alphaToPreviewPoint(*start);
  QPointF to = alphaToPreviewPoint(*end);
  from.setX(0);
  to.setX(clickableWidth());

  // Draw line through points
  path.moveTo(from);
  for (const auto& point : alphaManager_) path.lineTo(alphaToPreviewPoint(point));
  path.lineTo(to);

  alphaPath_->setPath(path);
  alphaPath_->setZValue(0);
  scene()->addItem(alphaPath_.get());
}

double ColormapPreview::clickableWidth() const
{
  return width() - BORDER_SIZE_;
}

double ColormapPreview::clickableHeight() const
{
  return height() - BORDER_SIZE_;
}

void ColormapPreview::deletePointItems()
{
  for (auto& item : scene()->items())
    if (dynamic_cast<QGraphicsEllipseItem*>(item))
      scene()->removeItem(item);

  alphaPath_.reset();
}

void ColormapPreview::redraw()
{
  deletePointItems();
  if (alphaManager_.empty())
    addDefaultLine();
  else
    for (const auto& point : alphaManager_)
      scene()->addItem(new ColorMapPreviewPoint(alphaToPreviewPoint(point)));

  drawAlphaPolyline();
}

void ColormapPreview::clearAlphaPointGraphics()
{
  alphaManager_.clear();
  pushToState();

  deletePointItems();
  removeCurrentLine();
  addDefaultLine();
  drawAlphaPolyline();
}
