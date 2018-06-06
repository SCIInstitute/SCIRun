// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include <algorithm>
#include "VolumeViewer.h"
//#include "TransferFunctionEditor.h"
#include "IsosurfaceEditor.h"
#include "SliceEditor.h"
#include "ProbeWidget.h"
#include "OpenGLAnnotationRenderer.h"
#include <components/ospcommon/FileName.h>

#include "importer/Importer.h"

using namespace ospcommon;

VolumeViewer::VolumeViewer(const OsprayViewerParameters& params, const OsprayGUIParameters& guiParams,
  const OsprayObjectParameters& objParams,
  QWidget* parent)
  : QWidget(parent),
    additionalObjects_(objParams.moreObjects),
    modelIndex(0),
    ownModelPerObject_(params.ownModelPerObject),
    boundingBox_(ospcommon::vec3f(0.f), ospcommon::vec3f(1.f)),
    presetBoundingBox_(objParams.presetBoundingBox),
    renderer(nullptr),
    rendererInitialized(false),
    transferFunction(nullptr),
    ambientLight(nullptr),
    directionalLight(nullptr),
    osprayWindow_(nullptr),
    annotationRenderer(nullptr),
    //transferFunctionEditor(nullptr),
    isosurfaceEditor(nullptr),
    autoRotationRate(0.025f),
    usePlane(-1),
    samplingRate(-1),
    adaptiveMaxSamplingRate(-1),
    spp(-1),
    shadows(-1),
    preIntegration(-1),
    aoSamples(-1),
    adaptiveSampling(-1),
    gradientShadingEnabled(-1)
{
  setLayout(new QVBoxLayout);

  // Default window size.
  resize(guiParams.height, guiParams.width);
  parent->resize(guiParams.height, guiParams.width);

  // Create and configure the OSPRay state.
  initObjects(params.rendererType);

  postInitObjectConstruction(params, guiParams);
}

VolumeViewer::~VolumeViewer()
{
  ospRelease(renderer);
  ospRelease(planeMesh);
  ospRelease(ambientLight);
  ospRelease(directionalLight);
  ospRelease(transferFunction);
  for (auto& m : modelStates_)
    m.release();
  for (auto& obj : additionalObjects_)
    ospRelease(obj);
}

#if 0
void VolumeViewer::setParameters(const OsprayObjectParameters& params)
{
  additionalObjects_ = params.moreObjects;
  qDebug() << "additionalObjects_" << additionalObjects_.size();
  presetBoundingBox_ = params.presetBoundingBox;

  loadAdditionalGeometries();
  initPostObjects();

  osprayWindow_->setWorldBounds(boundingBox_);
}
#endif

void VolumeViewer::postInitObjectConstruction(const OsprayViewerParameters& params, const OsprayGUIParameters& guiParams)
{
  // Create an OSPRay window and set it as the central widget, but don't let it start rendering until we're done with setup.
  osprayWindow_ = new QOSPRayWindow(renderer, params.writeFramesFilename, this, guiParams.frameRateWidget);
  layout()->addWidget(osprayWindow_);

  //PRINT(boundingBox_);
  // Set the window bounds based on the OSPRay world bounds.
  osprayWindow_->setWorldBounds(boundingBox_);

  // Connect the "play timesteps" timer.
  connect(&playTimeStepsTimer, SIGNAL(timeout()), this, SLOT(nextTimeStep()));
  {
    //TODO: connect to new config widgets
    lightEditor = new LightEditor(ambientLight, directionalLight,
      guiParams.ambientLightIntensitySpinBox, guiParams.directionalLightIntensitySpinBox, guiParams.directionalLightAzimuthSlider, guiParams.directionalLightElevationSlider);
    connect(lightEditor, SIGNAL(lightsChanged()), this, SLOT(render()));
    probeWidget = new ProbeWidget(this);
  }

  if (params.fullScreen)
    setWindowState(windowState() | Qt::WindowFullScreen);

  setGradientShadingEnabled(true);
  setAOSamples(1);
  setAdaptiveSampling(true);
  setPreIntegration(true);
  setShadows(true);
  setSPP(1);
  setPlane(true);
}

ospcommon::box3f VolumeViewer::getBoundingBox()
{
  return boundingBox_;
}

QOSPRayWindow *VolumeViewer::getWindow()
{
  return osprayWindow_;
}
//
// TransferFunctionEditor *VolumeViewer::getTransferFunctionEditor()
// {
//   return transferFunctionEditor;
// }

void VolumeViewer::setModel(size_t index)
{
  modelIndex = index;

  // Set current model on the OSPRay renderer.
  ospSetObject(renderer, "model", modelStates_[index].model);
  ospCommit(renderer);
  rendererInitialized = true;

  //PRINT(modelStates_[index].volumes.size());
  if (!modelStates_[index].volumes.empty())
  {
    // Update transfer function and isosurface editor data value range with the voxel range of the current model's first volume.
    OSPVolume volume = modelStates_[index].volumes[0]->handle;
    ospcommon::vec2f voxelRange = modelStates_[index].volumes[0]->voxelRange;

    if(voxelRange != ospcommon::vec2f(0.f))
    {
      //transferFunctionEditor->setDataValueRange(voxelRange);
      isosurfaceEditor->setDataValueRange(voxelRange);
    }

    // Update active volume on probe widget.
    probeWidget->setVolume(modelStates_[index].volumes[0]->handle);

#if 0
    // Update current filename information label.
    if (ownModelPerObject_)
      currentFilenameInfoLabel.setText("<b>Timestep " + QString::number(index) + QString("</b>: Data value range: [") + QString::number(voxelRange.x) + ", " + QString::number(voxelRange.y) + "]");
    else
      currentFilenameInfoLabel.setText("<b>Timestep " + QString::number(index) + QString("</b>: ") + QString(objectFileFilenames_[index].c_str()).split('/').back() + ". Data value range: [" + QString::number(voxelRange.x) + ", " + QString::number(voxelRange.y) + "]");
#endif
  }
  // Enable rendering on the OSPRay window.
  osprayWindow_->setRenderingEnabled(true);
}

std::string VolumeViewer::toString() const
{
  return "VolumeViewer";
}

void VolumeViewer::autoRotate(bool set)
{
  if (!osprayWindow_)
    return;

  if (set)
  {
    osprayWindow_->setRotationRate(autoRotationRate);
    osprayWindow_->updateGL();
  }
  else
  {
    osprayWindow_->setRotationRate(0.);
  }
}

void VolumeViewer::setAutoRotationRate(double rate)
{
  autoRotationRate = rate;
}

void VolumeViewer::nextTimeStep()
{
  modelIndex = (modelIndex + 1) % modelStates_.size();
  setModel(modelIndex);
  render();
}

void VolumeViewer::playTimeSteps(bool animate)
{
  if (animate)
    playTimeStepsTimer.start(500);
  else
    playTimeStepsTimer.stop();
}

void VolumeViewer::addSlice(std::string filename)
{
  sliceEditor->addSlice(filename);
}

void VolumeViewer::setShowFrameRate(bool on)
{
  if (osprayWindow_)
    osprayWindow_->setShowFrameRate(on);
}

void VolumeViewer::addGeometry(std::string filename)
{
#if 0
  // For now we assume PLY geometry files. Later we can support other geometry formats.

  // Get filename if not specified.
  if(filename.empty())
    filename = QFileDialog::getOpenFileName(this, tr("Load geometry"), ".", "Geometry files (*)").toStdString();

  if(filename.empty())
    return;

  // Attempt to load the geometry through the TriangleMeshFile loader.
  // OSPGeometry triangleMesh = ospNewGeometry("trianglemesh");

  // If successful, commit the triangle mesh and add it to all models.
  // if(TriangleMeshFile::importTriangleMesh(filename, triangleMesh) != NULL) {
  ospcommon::FileName fn = filename;
  ospray::miniSG::Model* msgModel = new miniSG::Model;
  bool loadedSGScene = false;
  if (fn.ext() == "stl") {
    miniSG::importSTL(*msgModel,fn);
    loadedSGScene = true;
  } else if (fn.ext() == "msg") {
    miniSG::importMSG(*msgModel,fn);
    loadedSGScene = true;
  } else if (fn.ext() == "tri") {
    miniSG::importTRI(*msgModel,fn);
    loadedSGScene = true;
  } else if (fn.ext() == "xml") {
    miniSG::importRIVL(*msgModel,fn);
    loadedSGScene = true;
  } else if (fn.ext() == "obj") {
    miniSG::importOBJ(*msgModel,fn);
    loadedSGScene = true;
  } else if (fn.ext() == "hbp") {
    miniSG::importHBP(*msgModel,fn);
    loadedSGScene = true;
  } else if (fn.ext() == "x3d") {
    miniSG::importX3D(*msgModel,fn);
    loadedSGScene = true;
  } else if (fn.ext() == "astl") {
        // miniSG::importSTL(msgAnimation,fn);
        // loadedSGScene = true;
  } else {
    ospray::importer::Group *newStuff = ospray::importer::import(filename);
    if (!newStuff) return;
    if (newStuff->geometry.size() != 1) return;

    OSPGeometry triangleMesh = newStuff->geometry[0]->handle;
  // For now: if this is a DDS geometry, assume it is a horizon and its color should be mapped through the first volume's transfer function.
    if(QString(filename.c_str()).endsWith(".dds") && modelStates_.size() > 0 && modelStates_[0].volumes.size() > 0) {

      OSPMaterial material = ospNewMaterial(renderer, "default");
      ospSet3f(material, "Kd", 1,1,1);
      ospSetObject(material, "volume", modelStates_[0].volumes[0]->handle);
      ospCommit(material);

      ospSetMaterial(triangleMesh, material);
      ospCommit(triangleMesh);

  // Create an instance of the geometry and add the instance to the main model(s)--this prevents the geometry
  // from being rebuilt every time the main model is committed (e.g. when slices / isosurfaces are manipulated)
      OSPModel modelInstance = ospNewModel();
      ospAddGeometry(modelInstance, triangleMesh);
      ospCommit(modelInstance);

      ospcommon::affine3f xfm = ospcommon::one;
      OSPGeometry triangleMeshInstance = ospNewInstance(modelInstance, (osp::affine3f&)xfm);
      ospCommit(triangleMeshInstance);

      for(size_t i=0; i<modelStates_.size(); i++) {
        ospAddGeometry(modelStates_[i].model, triangleMeshInstance);
        ospCommit(modelStates_[i].model);
      }
    }
  }
  if (loadedSGScene)
  {
    std::vector<OSPModel> instanceModels;

    for (size_t i = 0; i < msgModel->mesh.size(); i++) {
      Ref<miniSG::Mesh> msgMesh = msgModel->mesh[i];
      TriangleMeshSceneParser parser(ospray::cpp::Renderer(), "triangles");
      auto ospMesh = parser.createOSPRayGeometry(msgModel, msgMesh.ptr);

      OSPMaterial mat = ospNewMaterial(renderer, "OBJMaterial");
      ospSet3f(mat,"Kd",.8f,.8f,.8f);
      ospCommit(mat);
      ospSetMaterial(ospMesh.handle(), mat);
      ospCommit(ospMesh.handle());

      cpp::Model model_i;
      model_i.addGeometry(ospMesh);
      model_i.commit();
      instanceModels.push_back(model_i.handle());
    }

    for (size_t i = 0; i < msgModel->instance.size(); i++) {
      msgModel->instance[i].xfm = msgModel->instance[i].xfm*ospcommon::affine3f::translate(ospcommon::vec3f(16,16,.1));  // hack for landing gear
      OSPGeometry inst =
      ospNewInstance(instanceModels[msgModel->instance[i].meshID],
        reinterpret_cast<osp::affine3f&>(msgModel->instance[i].xfm));
      ospCommit(inst);
        // sceneModel->addGeometry(inst);
      for(size_t i=0; i<modelStates_.size(); i++) {
        ospAddGeometry(modelStates_[i].model, inst);
        ospCommit(modelStates_[i].model);
      }
    }
  }


  // Force render.
  render();
#endif
}

void VolumeViewer::screenshot(const QString& file)
{
  // Print current camera view parameters (can be used on command line to recreate view)
  qDebug() << "screenshot view parameters (use on command line to reproduce view): \n"
            << "  " << osprayWindow_->getViewport()->toString();

  QString filename(file);
  // Get filename if not specified.
  if (filename.isEmpty())
    filename = QFileDialog::getSaveFileName(this, tr("Save screenshot"), ".", "PNG files (*.png)");

  if (filename.isEmpty())
    return;

  // Make sure the filename has the proper extension.
  if (!filename.endsWith(".png"))
    filename += ".png";

  // Grab the image.
  auto image = osprayWindow_->grabFrameBuffer();

  // Save the screenshot.
  bool success = image.save(filename);

  qDebug() << (success ? "saved screenshot to " : "failed saving screenshot ") << filename;
}

void VolumeViewer::keyPressEvent(QKeyEvent * event)
{
  if (event->key() == Qt::Key_Escape)
  {
    close();
  }
  else if (event->key() == Qt::Key_P)
  {
    std::cout << "View parameters (use on command line to reproduce view): " << std::endl
            << "  " << *(osprayWindow_->getViewport()) << std::endl;
  }
  else if (event->key() == Qt::Key_L)
  {
    std::cout << "Light parameters (use on command line to reproduce view): " << std::endl
            << "  " << *lightEditor << std::endl;
  }
}

void VolumeViewer::commitVolumes()
{
  for(size_t i=0; i<modelStates_.size(); i++)
    for(size_t j=0; j<modelStates_[i].volumes.size(); j++)
      ospCommit(modelStates_[i].volumes[j]->handle);
}

void VolumeViewer::render()
{
  if (osprayWindow_)
  {
    osprayWindow_->resetAccumulationBuffer();
    osprayWindow_->updateGL();
  }
}

void VolumeViewer::setRenderAnnotationsEnabled(bool value)
{
  if (value)
  {
    if (!annotationRenderer)
      annotationRenderer = new OpenGLAnnotationRenderer(this);

    connect(osprayWindow_, SIGNAL(renderGLComponents()), annotationRenderer, SLOT(render()), Qt::UniqueConnection);
  }
  else
  {
    delete annotationRenderer;
    annotationRenderer = NULL;
  }

  render();
}

void VolumeViewer::setSubsamplingInteractionEnabled(bool value)
{
  ospSet1i(renderer, "spp", value ? -1 : 1);
  if(rendererInitialized)
    ospCommit(renderer);
}

void VolumeViewer::setGradientShadingEnabled(bool value)
{
  if (gradientShadingEnabled != value)
  {
    for(size_t i=0; i<modelStates_.size(); i++)
      for(size_t j=0; j<modelStates_[i].volumes.size(); j++) {
        ospSet1i(modelStates_[i].volumes[j]->handle, "gradientShadingEnabled", value);
        ospCommit(modelStates_[i].volumes[j]->handle);
      }

      render();
      gradientShadingEnabled = value;
    }
}

//! Set gradient shading flag on all volumes.
void VolumeViewer::setPreIntegration(bool value)
{
  if (preIntegration != value)
  {
    ospSet1i(transferFunction, "preIntegration", value);
    ospCommit(transferFunction);

    render();
    preIntegration = value;
  }
}

//! Set gradient shading flag on all volumes.
void VolumeViewer::setSingleShade(bool value)
{
  for(size_t i=0; i<modelStates_.size(); i++)
    for(size_t j=0; j<modelStates_[i].volumes.size(); j++) {
      ospSet1i(modelStates_[i].volumes[j]->handle, "singleShade", value);
      ospCommit(modelStates_[i].volumes[j]->handle);
  }

  render();
}

void VolumeViewer::setShadows(bool value)
{
  if (shadows != value)
  {
    ospSet1i(renderer, "shadowsEnabled", value);
    if (rendererInitialized)
      ospCommit(renderer);

    render();
    shadows = value;
  }
}

void VolumeViewer::setPlane(bool st)
{
  if (usePlane != st)
  {
    usePlane = st;
    if (planeMesh)
    {
      for(size_t i=0; i<modelStates_.size(); i++)
      {
        ospCommit(modelStates_[i].model);
        if (usePlane)
          ospAddGeometry(modelStates_[i].model, planeMesh);
        else
          ospRemoveGeometry(modelStates_[i].model, planeMesh);
        ospCommit(modelStates_[i].model);
      }
    }
    render();
  }
}

void VolumeViewer::setAOWeight(double value)
{
  ospSet1f(renderer, "aoWeight", value);
  if(rendererInitialized)
    ospCommit(renderer);
  render();
}

void VolumeViewer::setAOSamples(int value)
{
  if (aoSamples != value)
  {
    ospSet1i(renderer, "aoSamples", value);
    if(rendererInitialized)
      ospCommit(renderer);
    render();
    aoSamples = value;
  }
}

void VolumeViewer::setSPP(int value)
{
  if (spp != value)
  {
    ospSet1i(renderer, "spp", value);
    if(rendererInitialized)
      ospCommit(renderer);
    render();
    spp = value;
  }
}

//! Set gradient shading flag on all volumes.
void VolumeViewer::setAdaptiveScalar(double value)
{
  for(size_t i=0; i<modelStates_.size(); i++)
    for(size_t j=0; j<modelStates_[i].volumes.size(); j++) {
      ospSet1f(modelStates_[i].volumes[j]->handle, "adaptiveScalar", value);
      ospCommit(modelStates_[i].volumes[j]->handle);
  }
  render();
}


//! Set gradient shading flag on all volumes.
void VolumeViewer::setAdaptiveMaxSamplingRate(double value)
{
  if (adaptiveMaxSamplingRate != value)
  {
    for(size_t i=0; i<modelStates_.size(); i++)
      for(size_t j=0; j<modelStates_[i].volumes.size(); j++) {
        ospSet1f(modelStates_[i].volumes[j]->handle, "adaptiveMaxSamplingRate", value);
        ospCommit(modelStates_[i].volumes[j]->handle);
    }
    render();
    adaptiveMaxSamplingRate = value;
  }
}


//! Set gradient shading flag on all volumes.
void VolumeViewer::setAdaptiveBacktrack(double value)
{
  for(size_t i=0; i<modelStates_.size(); i++)
    for(size_t j=0; j<modelStates_[i].volumes.size(); j++) {
      ospSet1f(modelStates_[i].volumes[j]->handle, "adaptiveBacktrack", value);
      ospCommit(modelStates_[i].volumes[j]->handle);
  }

  render();
}

//! Set gradient shading flag on all volumes.
void VolumeViewer::setAdaptiveSampling(bool value)
{
  if (value != adaptiveSampling)
  {
    for(size_t i=0; i<modelStates_.size(); i++)
      for(size_t j=0; j<modelStates_[i].volumes.size(); j++) {
        ospSet1i(modelStates_[i].volumes[j]->handle, "adaptiveSampling", value);
        ospCommit(modelStates_[i].volumes[j]->handle);
      }

      render();
      adaptiveSampling = value;
  }
}

void VolumeViewer::setSamplingRate(double value)
{
  if (samplingRate != value)
  {
    for(size_t i=0; i<modelStates_.size(); i++)
    {
      for(size_t j=0; j<modelStates_[i].volumes.size(); j++)
      {
        ospSet1f(modelStates_[i].volumes[j]->handle, "samplingRate", value);
        ospCommit(modelStates_[i].volumes[j]->handle);
      }
    }
    render();
    samplingRate = value;
  }
}

void VolumeViewer::setVolumeClippingBox(ospcommon::box3f value)
{
  for(size_t i=0; i<modelStates_.size(); i++)
    for(size_t j=0; j<modelStates_[i].volumes.size(); j++)
    {
      ospSet3fv(modelStates_[i].volumes[j]->handle, "volumeClippingBoxLower", &value.lower.x);
      ospSet3fv(modelStates_[i].volumes[j]->handle, "volumeClippingBoxUpper", &value.upper.x);
      ospCommit(modelStates_[i].volumes[j]->handle);
    }

  render();
}

void VolumeViewer::setSlices(std::vector<SliceParameters> sliceParameters)
{
  // Provide the slices to OSPRay as the coefficients (a,b,c,d) of the plane equation ax + by + cz + d = 0.
  std::vector<ospcommon::vec4f> planes;

  for(size_t i=0; i<sliceParameters.size(); i++)
    planes.push_back(ospcommon::vec4f(sliceParameters[i].normal.x,
                                sliceParameters[i].normal.y,
                                sliceParameters[i].normal.z,
                                -dot(sliceParameters[i].origin, sliceParameters[i].normal)));

  OSPData planesData = ospNewData(planes.size(), OSP_FLOAT4, &planes[0].x);

  // Remove existing slice geometries from models.
  for (auto& state : modelStates_)
  {
    for(size_t j=0; j<state.slices.size(); j++)
    {
      ospRemoveGeometry(state.model, state.slices[j]->handle);
    }

    state.slices.clear();
  }

  // Add new slices for each volume of each model. Later we can do this only for the active model on time step change...
  for (auto& state : modelStates_)
  {
    if(planes.size() > 0)
    {
      for(size_t j=0; j<state.volumes.size(); j++)
      {
        OSPGeometry slicesGeometry = ospNewGeometry("slices");
        ospSetData(slicesGeometry, "planes", planesData);
        ospSetObject(slicesGeometry, "volume", state.volumes[j]->handle);
        ospCommit(slicesGeometry);

        ospAddGeometry(state.model, slicesGeometry);

        state.slices.push_back(std::make_shared<ModelState::Geometry>(slicesGeometry));
      }
    }

    ospCommit(state.model);
  }

  render();
}

void VolumeViewer::setIsovalues(const std::vector<float>& isovalues)
{
  // Remove existing isosurface geometries from models.
  for (auto& state : modelStates_)
  {
    for (const auto& iso : state.isosurfaces)
    {
      ospRemoveGeometry(state.model, iso->handle);
    }

    state.isosurfaces.clear();
  }

  OSPData isovaluesData = ospNewData(isovalues.size(), OSP_FLOAT, &isovalues[0]);

  // Add new isosurfaces for each volume of each model. Later we can do this only for the active model on time step change...
  for (auto& state : modelStates_)
  {
    if (isovalues.size() > 0)
    {
      for (const auto& vol : state.volumes)
      {
        OSPGeometry isosurfacesGeometry = ospNewGeometry("isosurfaces");
        ospSetData(isosurfacesGeometry, "isovalues", isovaluesData);
        ospSetObject(isosurfacesGeometry, "volume", vol->handle);
        ospCommit(isosurfacesGeometry);

        ospAddGeometry(state.model, isosurfacesGeometry);

        state.isosurfaces.push_back(std::make_shared<ModelState::Geometry>(isosurfacesGeometry));
      }
    }

    ospCommit(state.model);
  }

  render();
}

void VolumeViewer::importObjectsFromFile(const std::string &filename)
{
  //PRINT(ownModelPerObject_);
  if (!ownModelPerObject_)
  {
    // Create an OSPRay model and its associated model state.
    modelStates_.push_back(ModelState(ospNewModel()));
  }

  std::unique_ptr<ospray::importer::Group> imported(ospray::importer::import(filename));
  assert(imported);

  //PRINT(imported->geometry.size());

  for (const auto& geom : imported->geometry)
  {
    loadGeometry(geom->handle);
  }

  for (const auto& vol : imported->volume)
  {
    loadVolume(vol->handle, vol->voxelRange, vol->bounds);
  }

  if (!ownModelPerObject_)
    ospCommit(modelStates_.back().model);
}

void VolumeViewer::loadGeometry(OSPGeometry geom)
{
  //PRINT(ownModelPerObject_);
  //PRINT(modelStates_.size());

  if (ownModelPerObject_)
    modelStates_.push_back(ModelState(ospNewModel()));

  ospCommit(geom);
  // Add the loaded geometry to the model.
  ospAddGeometry(modelStates_.back().model, geom);

  if (ownModelPerObject_)
    ospCommit(modelStates_.back().model);

  //PRINT(modelStates_.size());
}

void VolumeViewer::loadVolume(OSPVolume vol, const vec2f& voxelRange, const box3f& bounds)
{
  if (ownModelPerObject_)
    modelStates_.push_back(ModelState(ospNewModel()));

  assert(vol);
  // For now we set the same transfer function on all volumes.
  ospSetObject(vol, "transferFunction", transferFunction);
  ospCommit(vol);

  // Add the loaded volume(s) to the model.
  ospAddVolume(modelStates_.back().model, vol);

  assert(!bounds.empty());
  // Add to volumes vector for the current model.
  modelStates_.back().volumes.push_back(std::make_shared<ModelState::Volume>(vol, bounds, voxelRange));

  if (ownModelPerObject_)
    ospCommit(modelStates_.back().model);
}

void VolumeViewer::initObjects(const std::string &renderer_type)
{
  globalInit(renderer_type);

  loadAdditionalGeometries();

  initPostObjects();
}

void VolumeViewer::loadAdditionalGeometries()
{
  if (!ownModelPerObject_)
  {
    modelStates_.push_back(ModelState(ospNewModel()));
  }

  for (const auto& geom : additionalObjects_)
  {
    loadGeometry(geom);
  }
}

void VolumeViewer::globalInit(const std::string &renderer_type)
{
  // Create an OSPRay renderer.
  renderer = ospNewRenderer(renderer_type.c_str());
  exitOnCondition(renderer == NULL, "could not create OSPRay renderer object");

  // Set renderer defaults (if not using 'aoX' renderers)
  if (renderer_type[0] != 'a' && renderer_type[1] != 'o')
  {
    ospSet1i(renderer, "aoSamples", 1);
    ospSet1i(renderer, "shadowsEnabled", 1);
    ospSet1i(renderer, "aoTransparencyEnabled", 1);
  }

  // Create OSPRay ambient and directional lights. GUI elements will modify their parameters.
  ambientLight = ospNewLight(renderer, "AmbientLight");
  exitOnCondition(ambientLight == NULL, "could not create ambient light");
  ospSet3f(ambientLight, "color", 0.3f, 0.5f, 1.f);
  ospCommit(ambientLight);

  directionalLight = ospNewLight(renderer, "DirectionalLight");
  exitOnCondition(directionalLight == NULL, "could not create directional light");
  ospSet3f(directionalLight, "color", 1.f, 0.9f, 0.4f);
  ospCommit(directionalLight);

  // Set the light sources on the renderer.
  std::vector<OSPLight> lights;
  lights.push_back(ambientLight);
  lights.push_back(directionalLight);

  ospSetData(renderer, "lights", ospNewData(lights.size(), OSP_OBJECT, &lights[0]));

  // Create an OSPRay transfer function.
  transferFunction = ospNewTransferFunction("piecewise_linear");
  exitOnCondition(transferFunction == NULL, "could not create OSPRay transfer function object");
  ospCommit(transferFunction);
}

const float b = -3;

void VolumeViewer::initPostObjects()
{
  boundingBox_ = ospcommon::empty;
  if (!modelStates_.empty())
  {
    for (const auto& vol : modelStates_[0].volumes)
      boundingBox_.extend(vol->boundingBox);

    if (modelStates_[0].volumes.empty())
    {
      boundingBox_ = presetBoundingBox_;
    }

    addInitialPlane();
  }
  else
  {
    boundingBox_ = {{b,b,b},{-b,-b,-b}};
  }

  osp::vec3f specular = osp::vec3f{0.135f,0.135f,0.135f};
  for (const auto& model : modelStates_)
  {
    for (const auto& vol : model.volumes)
    {
      ospSet3fv(vol->handle, "specular", &specular.x);
      ospCommit(vol->handle);
    }
  }
}

void VolumeViewer::addInitialPlane()
{
  OSPMaterial planeMaterial = ospNewMaterial(renderer,"default");
  ospSet3f(planeMaterial,"Kd",.5,.5,.5);
  ospSet3f(planeMaterial,"Ks",0,0,0);
  ospSet1f(planeMaterial,"Ns",0);
  ospCommit(planeMaterial);

  osp::vec3f vertices[4];
  float ps = 100000.f;
  float py = boundingBox_.upper.y+1.f;

  vertices[0] = osp::vec3f{-ps, -ps, py};
  vertices[1] = osp::vec3f{-ps,  ps, py};
  vertices[2] = osp::vec3f{ ps, -ps, py};
  vertices[3] = osp::vec3f{ ps,  ps, py};

  planeMesh = ospNewGeometry("triangles");
  OSPData position = ospNewData(4, OSP_FLOAT3, &vertices[0]);
  ospCommit(position);
  ospSetData(planeMesh, "vertex", position);

  osp::vec3i triangles[2];
  triangles[0] = osp::vec3i{0,1,2};
  triangles[1] = osp::vec3i{1,2,3};

  OSPData index = ospNewData(2, OSP_INT3, &triangles[0]);
  ospCommit(index);
  ospSetData(planeMesh, "index", index);

  ospSetMaterial(planeMesh, planeMaterial);
  ospCommit(planeMesh);
  setPlane(usePlane);
  ospRelease(index);
}

#if 0
void VolumeViewer::loadObjectsFromFiles()
{
  for (const auto& file : objectFileFilenames_)
    importObjectsFromFile(file);
}
#endif

void VolumeViewer::initUserInterfaceWidgets()
{


#if 0
  // Add the "add geometry" widget and callback.
  QAction *addGeometryAction = new QAction("Add geometry", this);
  connect(addGeometryAction, SIGNAL(triggered()), this, SLOT(addGeometry()));
  toolbar->addAction(addGeometryAction);

  // Create the transfer function editor dock widget, this widget modifies the transfer function directly.
  QDockWidget *transferFunctionEditorDockWidget = new QDockWidget("Transfer Function", this);
  transferFunctionEditor = new TransferFunctionEditor(transferFunction);
  transferFunctionEditorDockWidget->setWidget(transferFunctionEditor);
  connect(transferFunctionEditor, SIGNAL(committed()), this, SLOT(commitVolumes()));
  connect(transferFunctionEditor, SIGNAL(committed()), this, SLOT(render()));
  addDockWidget(Qt::LeftDockWidgetArea, transferFunctionEditorDockWidget);

  // Set the transfer function editor widget to its minimum allowed height, to leave room for other dock widgets.
  transferFunctionEditor->setMaximumHeight(transferFunctionEditor->minimumSize().height());

  // Create slice editor dock widget.
  QDockWidget *sliceEditorDockWidget = new QDockWidget("Slices", this);
  sliceEditor = new SliceEditor(boundingBox_);
  sliceEditorDockWidget->setWidget(sliceEditor);
  connect(sliceEditor, SIGNAL(slicesChanged(std::vector<SliceParameters>)), this, SLOT(setSlices(std::vector<SliceParameters>)));
  addDockWidget(Qt::LeftDockWidgetArea, sliceEditorDockWidget);

  // Create isosurface editor dock widget.
  QDockWidget *isosurfaceEditorDockWidget = new QDockWidget("Isosurfaces", this);
  isosurfaceEditor = new IsosurfaceEditor();
  isosurfaceEditorDockWidget->setWidget(isosurfaceEditor);
  connect(isosurfaceEditor, SIGNAL(isovaluesChanged(std::vector<float>)), this, SLOT(setIsovalues(std::vector<float>)));
  addDockWidget(Qt::LeftDockWidgetArea, isosurfaceEditorDockWidget);

  // Default to showing transfer function tab widget.
  transferFunctionEditorDockWidget->raise();
  // Tabify dock widgets.
  tabifyDockWidget(transferFunctionEditorDockWidget, sliceEditorDockWidget);
  tabifyDockWidget(transferFunctionEditorDockWidget, isosurfaceEditorDockWidget);
  tabifyDockWidget(transferFunctionEditorDockWidget, lightEditorDockWidget);
  tabifyDockWidget(transferFunctionEditorDockWidget, probeDockWidget);

  // Add the "screenshot" widget and callback.
  QAction *screenshotAction = new QAction("Screenshot", this);
  connect(screenshotAction, SIGNAL(triggered()), this, SLOT(screenshot()));
  toolbar->addAction(screenshotAction);

  // Create the light editor dock widget, this widget modifies the light directly.
  QDockWidget *lightEditorDockWidget = new QDockWidget("Lights", this);

  lightEditorDockWidget->setWidget(lightEditor);

  addDockWidget(Qt::LeftDockWidgetArea, lightEditorDockWidget);

  // Create the probe dock widget.
  QDockWidget *probeDockWidget = new QDockWidget("Probe", this);

  probeDockWidget->setWidget(probeWidget);
  addDockWidget(Qt::LeftDockWidgetArea, probeDockWidget);

  // Tabs on top.
  setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);

  // Add the current OSPRay object file label to the bottom status bar.
  statusBar()->addWidget(&currentFilenameInfoLabel);
#endif


}

void VolumeViewer::setAmbientLightVisible(bool visible)
{
  ospSet1i(ambientLight, "isVisible", visible);
  ospCommit(ambientLight);
  render();
}

void VolumeViewer::setDirectionalLightVisible(bool visible)
{
  ospSet1i(directionalLight, "isVisible", visible);
  ospCommit(directionalLight);
  render();
}

void VolumeViewer::setAmbientLightColor(float r, float g, float b)
{
  ospSet3f(ambientLight, "color", r, g, b);
  ospCommit(ambientLight);
  render();
}

void VolumeViewer::setDirectionalLightColor(float r, float g, float b)
{
  ospSet3f(directionalLight, "color", r, g, b);
  ospCommit(directionalLight);
  render();
}
