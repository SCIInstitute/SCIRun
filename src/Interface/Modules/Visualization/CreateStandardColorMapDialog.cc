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

#include <Interface/Modules/Visualization/CreateStandardColorMapDialog.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Modules/Visualization/CreateStandardColorMap.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Visualization;
using namespace SCIRun::Core::Datatypes;

typedef SCIRun::Modules::Visualization::CreateStandardColorMap CreateStandardColorMapModule;


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

  connect(shiftSpin_, SIGNAL(valueChanged(double)), this, SLOT(setShiftSlider(double)));
  connect(resolutionSpin_, SIGNAL(valueChanged(int)), this, SLOT(setResolutionSlider(int)));
  connect(shiftSpin_, SIGNAL(valueChanged(double)), this, SLOT(updateColorMapPreview()));
  connect(resolutionSpin_, SIGNAL(valueChanged(int)), this, SLOT(updateColorMapPreview()));

  connect(shiftSlider_, SIGNAL(valueChanged(int)), this, SLOT(setShiftSpinner(int)));
  connect(resolutionSlider_, SIGNAL(valueChanged(int)), resolutionSpin_, SLOT(setValue(int)));
  connect(invertCheck_, SIGNAL(toggled(bool)), this, SLOT(onInvertCheck(bool)));

  auto defaultMap = StandardColorMapFactory::create();
  auto rainboxIndex = colorMapNameComboBox_->findText("Rainbow", Qt::MatchExactly);
  if (rainboxIndex >= 0)
  {
    colorMapNameComboBox_->setCurrentIndex(rainboxIndex);
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

void CreateStandardColorMapDialog::pullSpecial()
{
  auto pointsVec = state_->getValue(Parameters::AlphaUserPointsVector).toVector();
  if (pointsVec.empty())
  {
    previewColorMap_->addDefaultLine();
  }
  else
  {
    previewColorMap_->addEndpoints();
    for (const auto& p : pointsVec)
    {
      auto pVec = p.toVector();
      previewColorMap_->addPoint(QPointF(pVec[0].toDouble(), pVec[1].toDouble()));
    }
  }
}

void CreateStandardColorMapDialog::updateColorMapPreview(const QString& s)
{
  auto cm = StandardColorMapFactory::create(s.toStdString(), resolutionSlider_->value(),
    static_cast<double>(shiftSlider_->value()) / 100.,
    invertCheck_->isChecked());
  previewColorMap_->setStyleSheet(buildGradientString(*cm));
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
  alphaFunction_(ALPHA_VECTOR_LENGTH, DEFAULT_ALPHA),
  dialogPulling_(pulling)
{
}

namespace
{
  const double colormapPreviewHeight = 83;
  const double colormapPreviewWidth = 365;
  const QRectF colorMapPreviewRect(0, 0, colormapPreviewWidth, colormapPreviewHeight);
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
  QGraphicsView::mousePressEvent(event);
  if (event->buttons() & Qt::LeftButton)
  {
    auto center = mapToScene(event->pos());
    addPoint(center);
  }

  //TODO: remove point if event & RightMouseButton

  //TODO: points are movable!
}

static QPen alphaLinePen(Qt::red, 1);

void ColormapPreview::addDefaultLine()
{
  removeDefaultLine();
  alphaPath_ = scene()->addLine(defaultStart_.x(), defaultStart_.y(),
    defaultEnd_.x(), defaultEnd_.y(),
    alphaLinePen);
  alphaManager_.insertEndpoints();
}

void AlphaFunctionManager::insertEndpoints()
{
  alphaPoints_.insert(defaultStart_);
  insert(defaultEnd_); // only update function and state after both endpoints are added
}

void ColormapPreview::removeDefaultLine()
{
  delete alphaPath_;
  alphaPath_ = nullptr;
}

void ColormapPreview::addPoint(const QPointF& point)
{
  if (alphaManager_.alreadyExists(point))
    return;

  removeDefaultLine();

  static QPen pointPen(Qt::white, 1);
  auto item = scene()->addEllipse(point.x() - 4, point.y() - 4, 8, 8, pointPen, QBrush(Qt::black));
  item->setZValue(1);
  // QString toolTip;
  // QDebug tt(&toolTip);
  // tt << "Alpha point " << point.x() << ", " << point.y() << " y% " << (1 - point.y() / sceneRect().height());
  // item->setToolTip(toolTip);
  alphaManager_.insert(point);

  drawAlphaPolyline();
}

bool AlphaFunctionManager::alreadyExists(const QPointF& point) const
{
  const double x = point.x();
  return std::find_if(alphaPoints_.begin(), alphaPoints_.end(), [=](const QPointF& p) { return p.x() == x; }) != alphaPoints_.end();
}

void AlphaFunctionManager::insert(const QPointF& p)
{
  alphaPoints_.insert(p);
  updateAlphaFunction();
  pushToState();
}

void AlphaFunctionManager::clear()
{
  alphaPoints_.clear();
  alphaFunction_.assign(ALPHA_VECTOR_LENGTH, DEFAULT_ALPHA);
  pushToState();
}

void AlphaFunctionManager::pushToState()
{
  if (!dialogPulling_)
  {
    Variable::List alphaPointsVec;

    //strip endpoints before saving user-added points
    auto begin = alphaPoints_.begin(), end = alphaPoints_.end();
    std::advance(begin, 1);
    std::advance(end, -1);
    std::for_each(begin, end, [&](const QPointF& p) { alphaPointsVec.emplace_back(Name("alphaPoint"), makeVariableList(p.x(), p.y())); });
    state_->setValue(Parameters::AlphaUserPointsVector, alphaPointsVec);

    state_->setTransientValue(Parameters::AlphaFunctionVector, alphaFunction_);
  }
}

void ColormapPreview::drawAlphaPolyline()
{
  removeDefaultLine();
  auto pathItem = new QGraphicsPathItem();
  alphaPath_ = pathItem;
  pathItem->setPen(alphaLinePen);
  QPainterPath path;
  QPointF from = defaultStart_;
  path.moveTo(from);

  for (const auto& point : alphaManager_)
  {
    path.lineTo(point);
    path.moveTo(point);
  }

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
  addDefaultLine();
}

void AlphaFunctionManager::updateAlphaFunction()
{
  //from v4, color endpoints (0 and 1) are fixed at alpha = 0.5.
  // alphaFunction_ will sample from in between these endpoints, evenly spaced throughout open interval (0,1)

  for (int i = 0; i < static_cast<int>(alphaFunction_.size()); ++i)
  {
    if (i > 0 && i < alphaFunction_.size() - 1)
    {
      double color = i / static_cast<double>(ALPHA_SAMPLES + 1);
      auto between = alphaLineEndpointsAtColor(color);
      alphaFunction_[i] = interpolateAlphaLineValue(between.first, between.second, color);
      // qDebug() << "Color: " << color << "Alpha: " << alphaFunction_[i] << "between points" << between.first << between.second;
    }
    else
    {
      alphaFunction_[i] = DEFAULT_ALPHA;
    }
  }
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
  return 1 - y / colorMapPreviewRect.height();
}

QPointF AlphaFunctionManager::colorToPoint(double color) const
{
  return QPointF(color * defaultEnd_.x(), 0);
}
