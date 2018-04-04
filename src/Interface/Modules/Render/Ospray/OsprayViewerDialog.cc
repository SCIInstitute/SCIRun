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
#include <Interface/Modules/Render/Ospray/OsprayViewerDialog.h>
#include <Interface/Modules/Render/Ospray/VolumeViewer.h>
#include <Core/Algorithms/Visualization/OsprayRenderAlgorithm.h>

using namespace SCIRun::Gui;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Algorithms::Visualization;

OsprayViewerDialog::OsprayViewerDialog(const std::string& name, ModuleStateHandle state,
  QWidget* parent /* = 0 */)
  : ModuleDialogGeneric(state, parent)
{
  OsprayRenderAlgorithm algo; // for ospray init


  //float dt = 0.0f;
  //std::vector<std::string> plyFilenames;
  //float rotationRate = 0.0f;
  //std::vector<std::string> sliceFilenames;
  //std::string transferFunctionFilename;
  //int benchmarkWarmUpFrames = 0;
  //int benchmarkFrames = 0;
  //std::string benchmarkFilename;
  //int viewSizeWidth = 0;
  //int viewSizeHeight = 0;
  //ospcommon::vec3f viewUp(0.f);
  //ospcommon::vec3f viewAt(0.f), viewFrom(0.f);
  bool showFrameRate = true;
  bool fullScreen = false;
  bool ownModelPerObject = false;
  std::string renderer = "scivis";//"dvr";
  //std::string writeFramesFilename;
  //bool usePlane = true;
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

  viewer_= new VolumeViewer({"/Users/dan/Downloads/csafe-heptane-302-volume/csafe-heptane-302-volume.osp"},
    showFrameRate,
    renderer,
    ownModelPerObject,
    fullScreen
  );

  
  viewer_->setModel(0);

  viewer_->getLightEditor()->setAmbientLightIntensity(ambientLightIntensity);
  viewer_->getLightEditor()->setDirectionalLightIntensity(directionalLightIntensity);
  viewer_->getLightEditor()->setDirectionalLightAzimuth(directionalLightAzimuth);
  viewer_->getLightEditor()->setDirectionalLightElevation(directionalLightElevation);

  viewer_->setSamplingRate(samplingRate);
  viewer_->setAdaptiveMaxSamplingRate(maxSamplingRate);
  viewer_->setAdaptiveSampling(adaptiveSampling);

  viewer_->setSPP(spp);
  viewer_->setShadows(!noshadows);
  viewer_->setAOSamples(aoSamples);
  viewer_->setPreIntegration(preIntegration);
  viewer_->setGradientShadingEnabled(gradientShading);

  viewer_->show();
}

OsprayViewerDialog::~OsprayViewerDialog()
{
  delete viewer_;
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
