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

#pragma once

#define NOMINMAX
#include <components/ospcommon/box.h>
#include <Interface/Modules/Render/Ospray/QOSPRayWindow.h>
#include <Interface/Modules/Render/Ospray/SliceWidget.h>
#include <Interface/Modules/Render/Ospray/LightEditor.h>
#include <QtGui>
#include <string>
#include <vector>

class TransferFunctionEditor;
class IsosurfaceEditor;
class ProbeWidget;
class OpenGLAnnotationRenderer;
class PreferencesDialog;

//! OSPRay model and its volumes / geometries
struct ModelState
{
  struct Volume
  {
    Volume(OSPVolume handle,
           const ospcommon::box3f &boundingBox,
           const ospcommon::vec2f &voxelRange)
      : handle(handle),
        boundingBox(boundingBox),
        voxelRange(voxelRange)
    {
      assert(!boundingBox.empty());
    }

    OSPVolume handle;
    ospcommon::vec2f voxelRange;
    ospcommon::box3f boundingBox;
  };

  using VolumePtr = std::shared_ptr<Volume>;

  struct Geometry
  {
    Geometry(OSPGeometry handle=NULL) : handle(handle) {}
    OSPGeometry handle;
  };

  using GeometryPtr = std::shared_ptr<Geometry>;

  ModelState(OSPModel model) : model(model) { }

  OSPModel model; //!< the OSPRay model

  std::vector<VolumePtr> volumes; //!< OSPRay volumes for the model
  std::vector<GeometryPtr> slices; //! OSPRay slice geometries for the model
  std::vector<GeometryPtr> isosurfaces; //! OSPRay isosurface geometries for the model
};

class VolumeViewer : public QMainWindow
{

Q_OBJECT

public:
  explicit VolumeViewer(const std::vector<std::string> &objectFileFilenames,
               bool showFrameRate,
               const std::string& renderer_type,
               bool ownModelPerObject,
               bool fullScreen,
               const std::vector<OSPGeometry>& moreObjects = {},
               const std::string& writeFramesFilename = "");

  ospcommon::box3f getBoundingBox();

  //! Get the OSPRay output window.
  QOSPRayWindow *getWindow();

  //! Get the transfer function editor.
  TransferFunctionEditor *getTransferFunctionEditor();

  //! Select the model (time step) to be displayed.
  void setModel(size_t index);

  //! A string description of this class.
  std::string toString() const;

  void loadObjectsFromFiles();
  void loadGeometry(OSPGeometry geom);
  void loadVolume(OSPVolume vol, const ospcommon::vec2f& voxelRange, const ospcommon::box3f& bounds);

public Q_SLOTS:

  //! Toggle auto-rotation of the view.
  void autoRotate(bool set);

  //! Set auto-rotation rate
  void setAutoRotationRate(float rate);

  //! Draw the model associated with the next time step.
  void nextTimeStep();

  //! Toggle animation over the time steps.
  void playTimeSteps(bool animate);

  //! Add a slice to the volume from file.
  void addSlice(std::string filename);

  //! Add geometry from file.
  void addGeometry(std::string filename = std::string());

  //! Save screenshot.
  void screenshot(std::string filename = std::string());

  //! Quit the volume viewer when pressing escape
  virtual void keyPressEvent(QKeyEvent * event);

  //! Re-commit all OSPRay volumes.
  void commitVolumes();

  //! Force the OSPRay window to be redrawn.
  void render();

  //! Enable / disable rendering of annotations.
  void setRenderAnnotationsEnabled(bool value);

  //! Set subsampling during interaction mode on renderer.
  void setSubsamplingInteractionEnabled(bool value);

  //! Set gradient shading flag on all volumes.
  void setGradientShadingEnabled(bool value);
  bool getGradientShadingEnabled() { return gradientShadingEnabled; }

  //! Set gradient shading flag on all volumes.
  void setPreIntegration(bool value);
  bool getPreIntegration() { return preIntegration; }

  //! Set gradient shading flag on all volumes.
  void setSingleShade(bool value);

  void setShadows(bool value);
  bool getShadows() { return shadows; }

  void setAdaptiveScalar(double value);

  void setAdaptiveMaxSamplingRate(double value);
  float getAdaptiveMaxSamplingRate() { return adaptiveMaxSamplingRate; }

  void setAdaptiveBacktrack(double value);

  //! Set gradient shading flag on all volumes.
  void setAdaptiveSampling(bool value);
  bool getAdaptiveSampling() { return adaptiveSampling; }

  //! Set sampling rate on all volumes.
  void setSamplingRate(double value);
  bool getSamplingRate() { return samplingRate; }

  //! Set volume clipping box on all volumes.
  void setVolumeClippingBox(ospcommon::box3f value);

  //! Set slices on all volumes.
  void setSlices(std::vector<SliceParameters> sliceParameters);

  //! Set isosurfaces on all volumes.
  void setIsovalues(const std::vector<float>& isovalues);

  void setPlane(bool st);
  bool getPlane() { return usePlane; }

  void setAOSamples(int value);
  int getAOSamples() { return aoSamples; }

  void setSPP(int value);
  int getSPP() { return spp; }

  void setAOWeight(double value);

  LightEditor* getLightEditor() { return lightEditor; }

protected:

  //! OSPRay object file filenames, one for each model / time step.
  std::vector<std::string> objectFileFilenames_;
  std::vector<OSPGeometry> additionalObjects_;
  bool ownModelPerObject_; // create a separate model for each object (not only for each file)

  //! OSPRay models and their volumes / geometries.
  std::vector<ModelState> modelStates_;

  //! Active OSPRay model index (time step).
  size_t modelIndex;

  //! Bounding box of the (first) volume.
  ospcommon::box3f boundingBox_;

  //! OSPRay renderer.
  OSPRenderer renderer;

  //! OSPRay renderer initialization state: set to true after renderer is committed.
  bool rendererInitialized;

  //! OSPRay transfer function.
  OSPTransferFunction transferFunction;

  //! OSPRay ambient light.
  OSPLight ambientLight;

  //! OSPRay directional light.
  OSPLight directionalLight;

  //! The OSPRay output window.
  QOSPRayWindow *osprayWindow;

  //! The OpenGL annotation renderer.
  OpenGLAnnotationRenderer *annotationRenderer;

  //! The transfer function editor.
  TransferFunctionEditor *transferFunctionEditor;

  //! The slice editor.
  SliceEditor *sliceEditor;

  //! The isosurface editor.
  IsosurfaceEditor *isosurfaceEditor;

  //! The probe widget.
  ProbeWidget *probeWidget;

  //! Auto-rotate button.
  QAction *autoRotateAction;

  //! Auto-rotation rate
  float autoRotationRate;

  //! Timer for use when stepping through multiple models.
  QTimer playTimeStepsTimer;

  //! Label for current OSPRay object file information.
  QLabel currentFilenameInfoLabel;

  //! Print an error message.
  void errorMessage(const std::string &kind, const std::string &message) const
  { std::cerr << "  " + toString() + "  " + kind + ": " + message + "." << std::endl; }

  //! Error checking.
  void exitOnCondition(bool condition, const std::string &message) const
  { if (!condition) return;  errorMessage("ERROR", message);  exit(1); }

  //! Load an OSPRay model from a file.
  void importObjectsFromFile(const std::string &filename);

  void postInitObjectConstruction(bool showFrameRate, const std::string& writeFramesFilename, bool fullScreen);
  //! Create and configure the OSPRay state.
  void initObjects(const std::string &renderer_type);
  void globalInit(const std::string &renderer_type);
  void initPostObjects();
  void addInitialPlane();
  void loadAdditionalGeometries();

  //! Create and configure the user interface widgets and callbacks.
  void initUserInterfaceWidgets();

  int usePlane;

  OSPGeometry planeMesh;

  LightEditor* lightEditor;
  float samplingRate;
  float adaptiveMaxSamplingRate;
  PreferencesDialog* preferencesDialog;
  int spp;
  int aoSamples;
  int shadows;
  int preIntegration;
  int adaptiveSampling;
  int gradientShadingEnabled;

};
