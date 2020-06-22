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

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Datatypes;

typedef SCIRun::Modules::Visualization::CreateStandardColorMap CreateStandardColorMapModule;

namespace
{
  const double colormapPreviewHeight = 83;
  const double colormapPreviewWidth = 365;
  const QRectF colorMapPreviewRect(0, 0, colormapPreviewWidth, colormapPreviewHeight);
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

  connect(shiftSpin_, SIGNAL(valueChanged(double)), this, SLOT(setShiftSlider(double)));
  connect(resolutionSpin_, SIGNAL(valueChanged(int)), this, SLOT(setResolutionSlider(int)));
  connect(shiftSpin_, SIGNAL(valueChanged(double)), this, SLOT(updateColorMapPreview()));
  connect(resolutionSpin_, SIGNAL(valueChanged(int)), this, SLOT(updateColorMapPreview()));

  connect(shiftSlider_, SIGNAL(valueChanged(int)), this, SLOT(setShiftSpinner(int)));
  connect(resolutionSlider_, SIGNAL(valueChanged(int)), resolutionSpin_, SLOT(setValue(int)));
  connect(invertCheck_, SIGNAL(toggled(bool)), this, SLOT(onInvertCheck(bool)));

  connect(customColorButton0_, SIGNAL(clicked()), this, SLOT(selectCustomColorMin()));
  connect(customColorButton1_, SIGNAL(clicked()), this, SLOT(selectCustomColorMax()));

  customColors_[0] = colorFromState(Parameters::CustomColor0);
  customColors_[1] = colorFromState(Parameters::CustomColor1);

  customColorButton0_->setVisible(false);
  customColorButton1_->setVisible(false);

  auto defaultMap = StandardColorMapFactory::create();
  auto rainbowIndex = colorMapNameComboBox_->findText("Rainbow", Qt::MatchExactly);
  if (rainbowIndex >= 0)
  {
    colorMapNameComboBox_->setCurrentIndex(rainbowIndex);
  }
  else
    qDebug() << "NO RAINBOW!";

  addComboBoxManager(colorMapNameComboBox_, Parameters::ColorMapName);
  connect(colorMapNameComboBox_, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(updateColorMapPreview(const QString&)));

  scene_ = new QGraphicsScene(this);
  previewColorMap_ = new ColormapPreview(scene_, state, pulling_, this);
  qobject_cast<QVBoxLayout*>(groupBox->layout())->insertWidget(0, previewColorMap_);
  previewColorMap_->setStyleSheet(buildGradientString(*defaultMap));
  previewColorMap_->setMinimumSize(100,40);
  previewColorMap_->show();
  connect(clearAlphaPointsToolButton_, SIGNAL(clicked()), previewColorMap_, SLOT(clearAlphaPointGraphics()));
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
  auto val = state_->getValue(Parameters::AlphaUserPointsVector).toVector();

  auto pointsVec = AlphaFunctionManager::convertPointsFromState(val);
  previewColorMap_->updateFromState(pointsVec);
}

void ColormapPreview::updateFromState(const LogicalAlphaPointSet& points)
{
  if (points.empty())
  {
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
}

static ColorRGB toColorRGB(QColor& in)
{
  return ColorRGB(in.red() / 255.0f, in.green() / 255.0f, in.blue() / 255.0f);
}

void CreateStandardColorMapDialog::updateColorMapPreview(const QString& s)
{
  ColorMapHandle cmap;
  if (s.toStdString() == "Custom")
  {
    customColorButton0_->setVisible(true);
    customColorButton1_->setVisible(true);
    cmap = StandardColorMapFactory::create({toColorRGB(customColors_[0]), toColorRGB(customColors_[1])},
             s.toStdString(), resolutionSlider_->value(),
             static_cast<double>(shiftSlider_->value()) / 100., invertCheck_->isChecked());
  }
  else
  {
    customColorButton0_->setVisible(false);
    customColorButton1_->setVisible(false);
    cmap = StandardColorMapFactory::create(s.toStdString(), resolutionSlider_->value(),
             static_cast<double>(shiftSlider_->value()) / 100., invertCheck_->isChecked());
  }

  StandardColorMapFactory::create();
  previewColorMap_->setStyleSheet(buildGradientString(*cmap));
}

void CreateStandardColorMapDialog::updateColorMapPreview()
{
  updateColorMapPreview(colorMapNameComboBox_->currentText());
}

QString CreateStandardColorMapDialog::buildGradientString(const ColorMap& cm) const
{
  //TODO: cache these values, GUI is slow to update.
  std::stringstream ss;
  ss << "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0,";
  for (double i = 0.001; i < 1.0; i += 0.001) { //styling values need to be in the range [0,1]
    ss << " stop:" << i;
    ss << " rgba(";
    ColorRGB c = cm.valueToColor(i * 2. - 1.); //need to match default ColorMap data range [-1,1]
    ss << int(255.*c.r()) << ", " << int(255.*c.g()) << ", " << int(255.*c.b()) << ", 255),";
  }
  ss << ");";
  std::string str = ss.str();
  return QString::fromStdString(ss.str());
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

void CreateStandardColorMapDialog::onInvertCheck(bool b)
{
  updateColorMapPreview();
}

AlphaFunctionManager::AlphaFunctionManager(const QPointF& start, const QPointF& end, ModuleStateHandle state, const boost::atomic<bool>& pulling) :
  state_(state),
  defaultStart_(start), defaultEnd_(end),
  dialogPulling_(pulling)
{
}

ColormapPreview::ColormapPreview(QGraphicsScene* scene, ModuleStateHandle state,
  const boost::atomic<bool>& pulling,
  QWidget* parent)
  : QGraphicsView(scene, parent), alphaPath_(nullptr),
  defaultStart_(0, colormapPreviewHeight / 2),
  defaultEnd_(colormapPreviewWidth, colormapPreviewHeight / 2),
  alphaManager_(defaultStart_, defaultEnd_, state, pulling),
  dialogPulling_(pulling)
{
  setSceneRect(colorMapPreviewRect);
}

void ColormapPreview::mousePressEvent(QMouseEvent* event)
{
  #if DEVLOG
  qDebug() << "\n\n~~~~~~~~~~~~~~~~~~pressed at" << event->pos();
  #endif

  QGraphicsView::mousePressEvent(event);

  if (event->buttons() & Qt::LeftButton)
  {
    auto center = mapToScene(event->pos());

    if (event->modifiers() == Qt::ShiftModifier)
      removePointAndUpdateLine(center);
    else
      addPointAndUpdateLine(center);
  }

  //TODO: remove point if event & RightMouseButton
  //TODO: points are movable!
}

void ColormapPreview::mouseMoveEvent(QMouseEvent* event)
{
  QGraphicsView::mouseMoveEvent(event);

  if (event->buttons() & Qt::LeftButton && event->modifiers() != Qt::ShiftModifier)
  {
    removeDefaultLine();
    drawAlphaPolyline();
  }
}

void ColormapPreview::mouseReleaseEvent(QMouseEvent* event)
{
  alphaManager_.pushToState();
}

static QPen alphaLinePen(Qt::red, 1);

void ColormapPreview::addDefaultLine()
{
  removeDefaultLine();
  alphaPath_ = scene()->addLine(defaultStart_.x(), defaultStart_.y(),
    defaultEnd_.x(), defaultEnd_.y(),
    alphaLinePen);
}

void ColormapPreview::removeDefaultLine()
{
  #if DEVLOG
  qDebug() << "alphaPath" << alphaPath_;
  #endif
  if (alphaPath_)
    scene()->removeItem(alphaPath_);
  delete alphaPath_;
  alphaPath_ = nullptr;
}

  ColorMapPreviewPoint::ColorMapPreviewPoint(qreal x, qreal y)
    : QGraphicsEllipseItem(x - 4, y - 4, 8, 8), center_(x, y)
  {
    setPen(QPen(Qt::white, 1));
    setBrush(QBrush(Qt::black));
    //setFlag(QGraphicsItem::ItemIsMovable, true);
    setZValue(1);
  }

void ColormapPreview::addPointAndUpdateLine(const QPointF& point)
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif

  if (alphaManager_.alreadyExists(point))
    return;

  removeDefaultLine();

  justAddPoint(point);
  drawAlphaPolyline();
}

void ColormapPreview::justAddPoint(const QPointF& point)
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif
  auto item = new ColorMapPreviewPoint(point.x(), point.y());
  scene()->addItem(item);
  alphaManager_.insert(item->center());
}

void ColormapPreview::updateLine()
{
  removeDefaultLine();
  drawAlphaPolyline();
}

void ColormapPreview::addPointsAndLineFromFile(const LogicalAlphaPointSet& pointsToLoad)
{
  for (const auto& p : pointsToLoad)
    justAddPoint(p);

  updateLine();
}

void ColormapPreview::removePointAndUpdateLine(const QPointF& point)
{
  #if DEVLOG
  qDebug() << __FUNCTION__ << point;
  #endif
  removeDefaultLine();

  #if DEVLOG
  qDebug() << "need to remove at" << point;
  #endif

  auto pts = scene()->items(point);

  #if DEVLOG
  qDebug() << pts;
  #endif

  ColorMapPreviewPoint* itemToRemove = nullptr;
  for (auto item : pts)
  {
    if (auto c = dynamic_cast<ColorMapPreviewPoint*>(item))
    {
      itemToRemove = c;
      break;
    }
  }
  if (itemToRemove)
  {
    scene()->removeItem(itemToRemove);
    alphaManager_.erase(itemToRemove->center());
    delete itemToRemove;
  }

  drawAlphaPolyline();
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

void AlphaFunctionManager::insert(const QPointF& p)
{
  alphaPoints_.insert(p);
  #if DEVLOG
  qDebug() << "inserting" << p;
  printSet();
  #endif
}

void AlphaFunctionManager::printSet() const
{
  std::for_each(alphaPoints_.begin(), alphaPoints_.end(), [](const QPointF& p) { qDebug() << '\t' << p; });
}

void AlphaFunctionManager::erase(const QPointF& p)
{
  alphaPoints_.erase(p);
  #if DEVLOG
  qDebug() << "erasing" << p;
  printSet();
  #endif
}

size_t AlphaFunctionManager::size() const
{
  return alphaPoints_.size();
}

void AlphaFunctionManager::clear()
{
  alphaPoints_.clear();
}

void AlphaFunctionManager::pushToState()
{
  if (!dialogPulling_)
  {
    if (!alphaPoints_.empty())
    {
      state_->setValue(Parameters::AlphaUserPointsVector, convertPointsToState(alphaPoints_));
    }
    else
    {
      state_->setValue(Parameters::AlphaUserPointsVector, Variable::List());
    }
  }
}

LogicalAlphaPointSet AlphaFunctionManager::convertPointsFromState(const Variable::List& statePoints)
{
  auto toQPointF = [](const Variable& v) { auto v2 = v.toVector(); return QPointF(v2[0].toDouble() * colormapPreviewWidth, (1.0 - v2[1].toDouble()) * colormapPreviewHeight); };
  auto pairVec = toTypedVector<QPointF>(statePoints, toQPointF);
  return LogicalAlphaPointSet(pairVec.begin(), pairVec.end());
}

Variable::List AlphaFunctionManager::convertPointsToState(const LogicalAlphaPointSet& points)
{
  Variable::List alphaPointsVec;
  auto begin = points.begin(), end = points.end();
  std::for_each(begin, end, [&](const QPointF& p) { alphaPointsVec.emplace_back(Name("alphaPoint"),
    makeAnonymousVariableList(p.x() / colormapPreviewWidth, 1.0f - p.y() / colormapPreviewHeight)); });
  return alphaPointsVec;
}

void ColormapPreview::drawAlphaPolyline()
{
  removeDefaultLine();
  if (alphaManager_.size() == 0) return;
  auto pathItem = new QGraphicsPathItem();
  alphaPath_ = pathItem;
  pathItem->setPen(alphaLinePen);

  QPainterPath path;
  auto start = alphaManager_.begin();
  auto end = alphaManager_.end(); std::advance(end, -1);
  QPointF from = QPointF(defaultStart_.x(), start->y());
  QPointF to = QPointF(defaultEnd_.x(), end->y());

  path.moveTo(from);
  for (const auto& point : alphaManager_) path.lineTo(point);
  path.lineTo(to);

  pathItem->setPath(path);
  pathItem->setZValue(0);
  scene()->addItem(alphaPath_);
}

void ColormapPreview::clearAlphaPointGraphics()
{
  for (auto& item : scene()->items())
  {
    if (dynamic_cast<QGraphicsEllipseItem*>(item))
      scene()->removeItem(item);
  }
  alphaManager_.clear();
  alphaManager_.pushToState();
  addDefaultLine();
}

std::pair<QPointF,QPointF> AlphaFunctionManager::alphaLineEndpointsAtColor(double color) const
{
  auto rightIter = alphaPoints_.upper_bound(colorToPoint(color));
  auto right = *rightIter;
  auto left = *(--rightIter);
  return {left, right};
}

double AlphaFunctionManager::interpolateAlphaLineValue(const QPointF& leftEndpoint, const QPointF& rightEndpoint, double color) const
{
  if (rightEndpoint.x() == leftEndpoint.x())
    return 0.5; //???

  const double slope = (rightEndpoint.y() - leftEndpoint.y()) / (rightEndpoint.x() - leftEndpoint.x());
  const double intercept = rightEndpoint.y() - slope * rightEndpoint.x();
  double alpha = pointYToAlpha(slope * colorToPoint(color).x() + intercept);

  return alpha;
}

double AlphaFunctionManager::pointYToAlpha(double y) const
{
  return 1.0f - y / colorMapPreviewRect.height();
}

QPointF AlphaFunctionManager::colorToPoint(double color) const
{
  return QPointF(color * defaultEnd_.x(), 0);
}
