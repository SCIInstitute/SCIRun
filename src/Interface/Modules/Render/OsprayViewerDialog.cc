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
#include <Interface/Modules/Render/OsprayViewerDialog.h>
#include <Core/Algorithms/Visualization/OsprayRenderAlgorithm.h>
#include <Interface/Modules/Render/ViewOspraySceneConfig.h>
#include <Modules/Render/ViewScene.h>
#include <Modules/Render/OsprayViewer.h>
#include <Core/Logging/Log.h>

#ifdef WITH_OSPRAY
#include <Interface/Modules/Render/Ospray/VolumeViewer.h>
#endif

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Render;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

#ifdef WITH_OSPRAY
namespace
{


    //float dt = 0.0f;   //EXPOSE
    //std::vector<std::string> plyFilenames;
    //float rotationRate = 0.0f;    //EXPOSE
    //std::vector<std::string> sliceFilenames;
    //std::string transferFunctionFilename;
    //int benchmarkWarmUpFrames = 0;
    //int benchmarkFrames = 0;
    //std::string benchmarkFilename;
    //int viewSizeWidth = 0;    //EXPOSE
    //int viewSizeHeight = 0;    //EXPOSE
    //ospcommon::vec3f viewUp(0.f);
    //ospcommon::vec3f viewAt(0.f), viewFrom(0.f);
    //std::string writeFramesFilename;     //EXPOSE
    //bool usePlane = true;

  void setupViewer(VolumeViewer* viewer)
  {
    float ambientLightIntensity = 0.2f;
    float directionalLightIntensity = 1.7f;
    float directionalLightAzimuth = 80;
    float directionalLightElevation = 65;
    float samplingRate = .125f;
    float maxSamplingRate = 2.f;
    int spp = 1;
    bool noshadows = false;
    int aoSamples = 1;
    bool preIntegration = true;
    bool gradientShading = true;
    bool adaptiveSampling = true;

    viewer->setModel(0);

    viewer->getLightEditor()->setAmbientLightIntensity(ambientLightIntensity);
    viewer->getLightEditor()->setDirectionalLightIntensity(directionalLightIntensity);
    viewer->getLightEditor()->setDirectionalLightAzimuth(directionalLightAzimuth);
    viewer->getLightEditor()->setDirectionalLightElevation(directionalLightElevation);

    viewer->setSamplingRate(samplingRate);
    viewer->setAdaptiveMaxSamplingRate(maxSamplingRate);
    viewer->setAdaptiveSampling(adaptiveSampling);

    viewer->setSPP(spp);
    viewer->setShadows(!noshadows);
    viewer->setAOSamples(aoSamples);
    viewer->setPreIntegration(preIntegration);
    viewer->setGradientShadingEnabled(gradientShading);
  }

  OSPGeometry duplicatedCodeFromAlgorithm(OsprayGeometryObjectHandle obj)
  {
    const auto& fieldData = obj->data;
    const auto& vertex = fieldData.vertex;
    const auto& color = fieldData.color;
    const auto& index = fieldData.index;

    // create and setup model and mesh
    OSPGeometry mesh = ospNewGeometry("triangles");
    OSPData data = ospNewData(vertex.size() / 4, OSP_FLOAT3A, &vertex[0]); // OSP_FLOAT3 format is also supported for vertex positions
    ospCommit(data);
    ospSetData(mesh, "vertex", data);
    data = ospNewData(color.size() / 4, OSP_FLOAT4, &color[0]);
    ospCommit(data);
    ospSetData(mesh, "vertex.color", data);
    data = ospNewData(index.size() / 3, OSP_INT3, &index[0]); // OSP_INT4 format is also supported for triangle indices
    ospCommit(data);
    ospSetData(mesh, "index", data);
    return mesh;
  }

  ospcommon::box3f toOsprayBox(const BBox& box)
  {
    auto min = box.get_min();
    auto max = box.get_max();
    return {
      { static_cast<float>(min.x()),
        static_cast<float>(min.y()),
        static_cast<float>(min.z())},
      { static_cast<float>(max.x()),
        static_cast<float>(max.y()),
        static_cast<float>(max.z())}
    };
  }
}

class OsprayObjectImpl
{
public:
  std::vector<OSPGeometry> geoms_;
};
#else
class OsprayObjectImpl
{
};
#endif

OsprayViewerDialog::OsprayViewerDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent),
  impl_(new OsprayObjectImpl)
{
  setupUi(this);
  setWindowTitle(QString::fromStdString(name));

  SCIRun::Core::Algorithms::Visualization::OsprayRenderAlgorithm algo; // for ospray init

  state->connectSpecificStateChanged(Parameters::GeomData, [this]() { Q_EMIT newGeometryValueForwarder(); });
  connect(this, SIGNAL(newGeometryValueForwarder()), this, SLOT(newGeometryValue()));

  addConfigurationDialog();
  addToolBar();

  setMinimumSize(200, 200);

  addCheckBoxManager(configDialog_->showPlaneCheckBox_, Parameters::ShowPlane);
  addDoubleSpinBoxManager(configDialog_->autoRotationRateDoubleSpinBox_, Parameters::AutoRotationRate);

}

void OsprayViewerDialog::newGeometryValue()
{
#ifdef WITH_OSPRAY
  auto geomDataTransient = state_->getTransientValue(Parameters::GeomData);
  if (geomDataTransient && !geomDataTransient->empty())
  {
    auto geom = transient_value_cast<boost::shared_ptr<CompositeOsprayGeometryObject>>(geomDataTransient);
    if (!geom)
    {
      LOG_DEBUG("Logical error: ViewSceneDialog received an empty object.");
      return;
    }
    createViewer(*geom);
  }
#endif
}

void OsprayViewerDialog::createViewer(const CompositeOsprayGeometryObject& geom)
{
#ifdef WITH_OSPRAY
  delete viewer_;

  bool showFrameRate = false;
  bool fullScreen = false;
  bool ownModelPerObject = true;
  std::string renderer = "scivis";
  impl_->geoms_.clear();

  for (const auto& obj : geom.objects())
    impl_->geoms_.push_back(duplicatedCodeFromAlgorithm(obj));

  OsprayViewerParameters params
  {
    {},
    showFrameRate,
    renderer,
    ownModelPerObject,
    fullScreen,
    impl_->geoms_,
    toOsprayBox(geom.box),
    "",
    1024,
    768
  };
  viewer_ = new VolumeViewer(params, this);

  setupViewer(viewer_);

  {
    // TODO: need to move this to dialog ctor--create viewer once, and just change state.
    connect(configDialog_->autoRotationRateDoubleSpinBox_, SIGNAL(valueChanged(double)),
      viewer_, SLOT(setAutoRotationRate(double)));

    connect(configDialog_->showPlaneCheckBox_, SIGNAL(toggled(bool)),
      viewer_, SLOT(setPlane(bool)));
  }

  osprayLayout->addWidget(viewer_);
  viewer_->show();
#endif
}


// code from viewer main.cc
#if 0
// Default values for the optional command line arguments.

volumeViewer->setPlane(usePlane);

// Load PLY geometries from file.
for(unsigned int i=0; i<plyFilenames.size(); i++)
  volumeViewer->addGeometry(plyFilenames[i]);

// Set rotation rate to use in animation mode.
if(rotationRate != 0.f) {
  volumeViewer->setAutoRotationRate(rotationRate);
  volumeViewer->autoRotate(true);
}

if (dt > 0.0f)
  volumeViewer->setSamplingRate(dt);

// Load slice(s) from file.
for(unsigned int i=0; i<sliceFilenames.size(); i++)
  volumeViewer->addSlice(sliceFilenames[i]);

// Load transfer function from file.
if(transferFunctionFilename.empty() != true)
  volumeViewer->getTransferFunctionEditor()->load(transferFunctionFilename);

// Set benchmarking parameters.
volumeViewer->getWindow()->setBenchmarkParameters(benchmarkWarmUpFrames,
    benchmarkFrames, benchmarkFilename);

if (viewAt != viewFrom) {
  volumeViewer->getWindow()->getViewport()->at = viewAt;
  volumeViewer->getWindow()->getViewport()->from = viewFrom;
}

// Set the window size if specified.
if (viewSizeWidth != 0 && viewSizeHeight != 0) volumeViewer->getWindow()->setFixedSize(viewSizeWidth, viewSizeHeight);


// Set the view up vector if specified.
if(viewUp != ospcommon::vec3f(0.f)) {
  volumeViewer->getWindow()->getViewport()->setUp(viewUp);
  volumeViewer->getWindow()->resetAccumulationBuffer();
}
#endif

void OsprayViewerDialog::addToolBar()
{
  toolBar_ = new QToolBar(this);
  WidgetStyleMixin::toolbarStyle(toolBar_);

  addConfigurationButton();
  addAutoViewButton();
  addAutoRotateButton();
  addTimestepButtons();
  addScreenshotButton();

  osprayLayout->addWidget(toolBar_);

  //addViewBar();
  //addViewBarButton();
  addControlLockButton();
}

void OsprayViewerDialog::adjustToolbar()
{
  adjustToolbarForHighResolution(toolBar_);
}

void OsprayViewerDialog::addConfigurationButton()
{
  auto configurationButton = new QPushButton();
  configurationButton->setToolTip("Open/Close Configuration Menu");
  configurationButton->setIcon(QPixmap(":/general/Resources/ViewScene/configure.png"));
  configurationButton->setShortcut(Qt::Key_F5);
  connect(configurationButton, SIGNAL(clicked()), this, SLOT(configButtonClicked()));
  addToolbarButton(configurationButton);
}

void OsprayViewerDialog::configButtonClicked()
{
  configDialog_->setVisible(!configDialog_->isVisible());
}

void OsprayViewerDialog::addConfigurationDialog()
{
  auto name = windowTitle() + " Configuration";
  configDialog_ = new ViewOspraySceneConfigDialog(name, this);

  // configDialog_->setSampleColor(bgColor_);
  // configDialog_->setScaleBarValues(scaleBar_.visible, scaleBar_.fontSize, scaleBar_.length, scaleBar_.height,
  //   scaleBar_.multiplier, scaleBar_.numTicks, scaleBar_.visible, QString::fromStdString(scaleBar_.unit));
  // setupMaterials();
}

void OsprayViewerDialog::addToolbarButton(QPushButton* button)
{
  button->setFixedSize(35,35);
  button->setIconSize(QSize(25,25));
  toolBar_->addWidget(button);
}

void OsprayViewerDialog::addAutoViewButton()
{
  autoViewButton_ = new QPushButton(this);

  //TODO
  autoViewButton_->setDisabled(true);

  autoViewButton_->setToolTip("Auto View");
  autoViewButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  autoViewButton_->setShortcut(Qt::Key_0);
  connect(autoViewButton_, SIGNAL(clicked()), this, SLOT(autoViewClicked()));
  addToolbarButton(autoViewButton_);
}

void OsprayViewerDialog::addAutoRotateButton()
{
  autoRotateButton_ = new QPushButton(this);
  autoRotateButton_->setToolTip("Auto Rotate");
  autoRotateButton_->setCheckable(true);
  autoRotateButton_->setIcon(QPixmap(":/general/Resources/ViewScene/autorotate.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(autoRotateButton_, SIGNAL(clicked()), this, SLOT(autoRotateClicked()));
  addToolbarButton(autoRotateButton_);
}

void OsprayViewerDialog::addTimestepButtons()
{
  auto nextTimestep = new QPushButton(this);
  nextTimestep->setText("Next");
  nextTimestep->setToolTip("Next timestep");
  //autoRotateButton->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(nextTimestep, SIGNAL(clicked()), this, SLOT(nextTimestepClicked()));
  addToolbarButton(nextTimestep);

  playTimestepsButton_ = new QPushButton(this);
  playTimestepsButton_->setText("Play");
  playTimestepsButton_->setToolTip("Play timesteps");
  playTimestepsButton_->setCheckable(true);
  //autoRotateButton->setIcon(QPixmap(":/general/Resources/ViewScene/autoview.png"));
  //autoRotateButton->setShortcut(Qt::Key_0);
  connect(playTimestepsButton_, SIGNAL(clicked()), this, SLOT(playTimestepsClicked()));
  addToolbarButton(playTimestepsButton_);
}

void OsprayViewerDialog::addScreenshotButton()
{
  auto screenshotButton = new QPushButton(this);
  screenshotButton->setToolTip("Take screenshot");
  screenshotButton->setIcon(QPixmap(":/general/Resources/ViewScene/screenshot.png"));
  screenshotButton->setShortcut(Qt::Key_F12);
  connect(screenshotButton, SIGNAL(clicked()), this, SLOT(screenshotClicked()));
  addToolbarButton(screenshotButton);
}

void OsprayViewerDialog::addViewBarButton()
{
  auto viewBarBtn = new QPushButton();
  viewBarBtn->setToolTip("Show View Options");
  viewBarBtn->setIcon(QPixmap(":/general/Resources/ViewScene/views.png"));
  connect(viewBarBtn, SIGNAL(clicked()), this, SLOT(viewBarButtonClicked()));
  addToolbarButton(viewBarBtn);
}

void OsprayViewerDialog::addControlLockButton()
{
  controlLock_ = new QPushButton();

  //TODO
  controlLock_->setDisabled(true);

#if 0
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
  #endif
}

void OsprayViewerDialog::toggleLockColor(bool locked)
{
  QString color = locked ? "red" : "rgb(66,66,69)";
  controlLock_->setStyleSheet("QPushButton { background-color: " + color + "; }");
  //autoViewButton_->setDisabled(locked);
}

void OsprayViewerDialog::autoRotateClicked()
{
  if (viewer_)
    viewer_->autoRotate(autoRotateButton_->isChecked());
}

void OsprayViewerDialog::autoViewClicked()
{
  qDebug() << "TODO" << __FUNCTION__;
}

void OsprayViewerDialog::screenshotClicked()
{
  if (viewer_)
    viewer_->screenshot();
}

void OsprayViewerDialog::nextTimestepClicked()
{
  if (viewer_)
    viewer_->nextTimeStep();
}

void OsprayViewerDialog::playTimestepsClicked()
{
  if (viewer_)
    viewer_->playTimeSteps(playTimestepsButton_->isChecked());
}

#if 0
void OsprayViewerDialog::lockRotationToggled()
{
  mGLWidget->setLockRotation(lockRotation_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void OsprayViewerDialog::lockPanningToggled()
{
  mGLWidget->setLockPanning(lockPan_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void OsprayViewerDialog::lockZoomToggled()
{
  mGLWidget->setLockZoom(lockZoom_->isChecked());
  toggleLockColor(lockRotation_->isChecked() || lockPan_->isChecked() || lockZoom_->isChecked());
}

void OsprayViewerDialog::lockAllTriggered()
{
  lockRotation_->setChecked(true);
  mGLWidget->setLockRotation(true);
  lockPan_->setChecked(true);
  mGLWidget->setLockPanning(true);
  lockZoom_->setChecked(true);
  mGLWidget->setLockZoom(true);
  toggleLockColor(true);
}

void OsprayViewerDialog::unlockAllTriggered()
{
  lockRotation_->setChecked(false);
  mGLWidget->setLockRotation(false);
  lockPan_->setChecked(false);
  mGLWidget->setLockPanning(false);
  lockZoom_->setChecked(false);
  mGLWidget->setLockZoom(false);
  toggleLockColor(false);
}
#endif
