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


#ifndef INTERFACE_MODULES_VIEW_SCENE_H
#define INTERFACE_MODULES_VIEW_SCENE_H

/// \todo Make this definition specific to windows.
#define NOMINMAX

#include <Graphics/Datatypes/GeometryImpl.h>
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Render/ES/RendererInterfaceCollaborators.h>
#include <Interface/Modules/Render/ES/RendererInterfaceFwd.h>
#include <Interface/Modules/Render/ViewSceneControlsDock.h>
#include <Interface/Modules/Render/ViewSceneManager.h>
#include <Modules/Render/ViewScene.h>
#include <Modules/Visualization/TextBuilder.h>
#include <atomic>
#include "Interface/Modules/Render/ui_ViewScene.h"
#include <Interface/Modules/Render/share.h>

//TODO: needs to inherit from ModuleWidget somehow
class QToolBar;
class QStandardItemModel;
class QStandardItem;
class QGLWidget;

namespace SCIRun {
  namespace Gui {

    class GLWidget;
    class ViewSceneControlsDock;
    class ScopedWidgetColorChanger;
    class PreviousWidgetSelectionInfo;

    class SCISHARE ViewSceneDialog : public ModuleDialogGeneric, public Ui::ViewScene
    {
    Q_OBJECT;

    public:
      ViewSceneDialog(const std::string& name, Dataflow::Networks::ModuleStateHandle state,
        QWidget* parent = nullptr);
      ~ViewSceneDialog();

      std::string toString(std::string prefix) const;
      void adjustToolbar() override;

      static ViewSceneManager viewSceneManager;
      void inputMouseDownHelper(Render::MouseButton btn, float x, float y);
      void inputMouseMoveHelper(Render::MouseButton btn, float x, float y);
      void inputMouseUpHelper();
      void inputMouseWheelHelper(int32_t delta);
      void setViewScenesToUpdate(const std::unordered_set<ViewSceneDialog*>& scenes);
      std::string getName() {return name_;}
      void autoSaveScreenshot();

    Q_SIGNALS:
      void newGeometryValueForwarder();
      void cameraRotationChangeForwarder();
      void cameraLookAtChangeForwarder();
      void cameraDistnaceChangeForwarder();
      void lockMutexForwarder();
      void mousePressSignalForGeometryObjectFeedback(int x, int y, const std::string& selName);

    protected Q_SLOTS:
      void printToString() const {std::cout << toString("");}
      void sendBugReport();

      //---------------- New Geometry --------------------------------------------------------------
      void updateModifiedGeometries();
      void updateModifiedGeometriesAndSendScreenShot();
      void updateAllGeometries();
      void newGeometryValue(bool forceAllObjectsToUpdate);
      void sendGeometryFeedbackToState(int x, int y, const std::string& selName);
      void frameFinished();
      void lockMutex();
      void unblockExecution();
      void runDelayedGC();

      //---------------- Input ---------------------------------------------------------------------
      void viewBarButtonClicked();
      void configurationButtonClicked();
      void resizingDone();

      //---------------- Camera --------------------------------------------------------------------
      void autoViewClicked();
      void autoViewOnLoadChecked(bool value);
      void viewAxisSelected(const QString& name);
      void viewVectorSelected(const QString& name);
      void setFieldOfView(int value);
      void useOrthoViewChecked(bool value);
      void menuMouseControlChanged(int index);
      void invertZoomClicked(bool value);
      void adjustZoomSpeed(int value);
      void lockRotationToggled();
      void lockPanningToggled();
      void lockZoomToggled();
      void lockAllTriggered();
      void unlockAllTriggered();
      void toggleLockColor(bool locked);
      void stereoChecked(bool value);
      void setStereoFusion(int value);
      void setPolygonOffset(int value);
      void setTextOffset(int value);
      void setAutoRotateSpeed(double speed);
      void autoRotateRight();
      void autoRotateLeft();
      void autoRotateUp();
      void autoRotateDown();
      void pullCameraRotation();
      void pullCameraLookAt();
      void pullCameraDistance();

      //---------------- Widgets -------------------------------------------------------------------
      void updateMeshComponentSelection(const QString& moduleId, const QString& component, bool selected);

      //---------------- Clipping Planes -----------------------------------------------------------
      void setClippingPlaneIndex(int index);
      void setClippingPlaneVisible(bool value);
      void setClippingPlaneFrameOn(bool value);
      void reverseClippingPlaneNormal(bool value);
      void setClippingPlaneX(int index);
      void setClippingPlaneY(int index);
      void setClippingPlaneZ(int index);
      void setClippingPlaneD(int index);
      void useClipChecked(bool value);

      //---------------- Orietation Glyph ----------------------------------------------------------
      void showOrientationChecked(bool value);
      void setOrientAxisSize(int value);
      void setOrientAxisPosX(int pos);
      void setOrientAxisPosY(int pos);
      void setCenterOrientPos();
      void setDefaultOrientPos();

      //---------------- Scale Bar -----------------------------------------------------------------
      void setScaleBarVisible(bool value);
      void setScaleBarFontSize(int value);
      void setScaleBarUnitValue(const QString& text);
      void setScaleBarLength(double value);
      void setScaleBarHeight(double value);
      void setScaleBarMultiplier(double value);
      void setScaleBarNumTicks(int value);
      void setScaleBarLineWidth(double value);
      void setScaleBar();

      //---------------- Lights --------------------------------------------------------------------
      void setLightColor(int index);
      void toggleHeadLight(bool value);
      void setHeadLightAzimuth(int value);
      void setHeadLightInclination(int value);
      void toggleLight1(bool value);
      void setLight1Azimuth(int value);
      void setLight1Inclination(int value);
      void toggleLight2(bool value);
      void setLight2Azimuth(int value);
      void setLight2Inclination(int value);
      void toggleLight3(bool value);
      void setLight3Azimuth(int value);
      void setLight3Inclination(int value);
      void lightingChecked(bool value);

      //---------------- Material Settings ---------------------------------------------------------
      void setAmbientValue(double value);
      void setDiffuseValue(double value);
      void setSpecularValue(double value);
      void setShininessValue(double value);
      void setEmissionValue(double value);

      //---------------- Fog Tools -----------------------------------------------------------------
      void setFogOn(bool value);
      void setFogOnVisibleObjects(bool value);
      void setFogUseBGColor(bool value);
      void assignFogColor();
      void setFogStartValue(double value);
      void setFogEndValue(double value);

      //---------------- Misc. ---------------------------------------------------------------------
      void assignBackgroundColor();
      void setTransparencySortTypeContinuous(bool index);
      void setTransparencySortTypeUpdate(bool index);
      void setTransparencySortTypeLists(bool index);
      void screenshotClicked();
      void saveNewGeometryChanged(int state);
      void showBBoxChecked(bool value);
      void useBackCullChecked(bool value);
      void displayListChecked(bool value);


    protected:
      //---------------- Intitilization ------------------------------------------------------------
      void pullSpecial() override;

      //---------------- Input ---------------------------------------------------------------------
      void showEvent(QShowEvent* evt) override;
      void hideEvent(QHideEvent* evt) override;
      void resizeEvent(QResizeEvent *event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseReleaseEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;
      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent*event) override;
      void focusOutEvent(QFocusEvent* event) override;
      void focusInEvent(QFocusEvent* event) override;
      void closeEvent(QCloseEvent* evt) override;
      void contextMenuEvent(QContextMenuEvent* evt) override {}


    private:
      //---------------- Intitilization ------------------------------------------------------------
      void addToolBar();
      void setupClippingPlanes();
      void setupScaleBar();
      void setInitialLightValues();
      void setupMaterials();
      void setupRenderTabValues();
      void addAutoViewButton();
      void addScreenshotButton();
      void addViewBarButton();
      void addControlLockButton();
      void addToolbarButton(QPushButton* button);
      void addViewBar();
      void addViewOptions();
      void addConfigurationButton();
      void addConfigurationDock();
      QColor checkColorSetting(std::string& rgb, QColor defaultColor);
      void pullCameraState();
      void pushCameraDistance();
      void pushCameraLookAt();
      void pushCameraRotation();
      void pushCameraState();

      //---------------- Widgets -------------------------------------------------------------------
      bool needToWaitForWidgetSelection();
      bool canSelectWidget();
      bool tryWidgetSelection(int x, int y);
      void selectObject(const int x, const int y);
      Modules::Render::ViewScene::GeomListPtr getGeomData();
      bool checkForSelectedWidget(Graphics::Datatypes::WidgetHandle widget);
      void restoreObjColor();
      void backupColorValues(Graphics::Datatypes::WidgetHandle widget);
      void updateCursor();

      //---------------- Clipping Planes -----------------------------------------------------------
      void updatClippingPlaneDisplay();
      void buildGeomClippingPlanes();
      void buildGeometryClippingPlane(int index, const glm::vec4& plane, const Core::Geometry::BBox& bbox);

      //---------------- Scale Bar -----------------------------------------------------------------
      void updateScaleBarLength();
      Graphics::Datatypes::GeometryHandle buildGeometryScaleBar();

      //---------------- Lights --------------------------------------------------------------------
      void toggleLightOnOff(int index, bool value);

      //---------------- Materials -----------------------------------------------------------------
      void setMaterialFactor(Render::MatFactor factor, double value);

      //---------------- Fog -----------------------------------------------------------------------
      void setFog(Render::FogFactor factor, double value);
      void setFogColor(const glm::vec4 &color);

      //---------------- Misc. ---------------------------------------------------------------------
      void takeScreenshot();
      void sendScreenshotDownstreamForTesting();


      struct ClippingPlane
      {
        bool visible, showFrame, reverseNormal;
        double x, y, z, d;
      };

      struct ScaleBar
      {
        bool visible;
        int fontSize;
        double length, height, multiplier, numTicks, lineWidth;
        std::string unit;
        double projLength;
      };


      GLWidget*                             mGLWidget                     {nullptr};  ///< GL widget containing context.
      Render::RendererWeakPtr               mSpire                        {};         ///< Instance of Spire.
      QToolBar*                             mToolBar                      {nullptr};  ///< Tool bar.
      QToolBar*                             mViewBar                      {nullptr};  ///< Tool bar for view options.
      QComboBox*                            mDownViewBox                  {nullptr};  ///< Combo box for Down axis options.
      QComboBox*                            mUpVectorBox                  {nullptr};  ///< Combo box for Up Vector options.
      ViewSceneControlsDock*                mConfigurationDock            {nullptr};  ///< Dock holding configuration functions
      SharedPointer<ScopedWidgetColorChanger> widgetColorChanger_         {};
      PreviousWidgetSelectionInfo*          previousWidgetInfo_           {nullptr};

      bool                                  shown_                        {false};
      bool                                  delayGC                       {false};
      bool                                  delayedGCRequested            {false};
      bool                                  hideViewBar_                  {};
      bool                                  invertZoom_                   {};
      bool                                  shiftdown_                    {false};
      bool                                  mouseButtonPressed_           {false};
      Graphics::Datatypes::WidgetHandle     selectedWidget_;
      int                                   clippingPlaneIndex_           {0};
      const static int                      delayAfterModuleExecution_    {200};
      const static int                      delayAfterWidgetColorRestored_ {50};
      int                                   delayAfterLastSelection_      {50};
      float                                 clippingPlaneColors_[6][3]    {{0.7f, 0.2f, 0.1f}, {0.8f, 0.5f, 0.3f},
                                                                           {0.8f, 0.8f, 0.5f}, {0.4f, 0.7f, 0.3f},
                                                                           {0.2f, 0.4f, 0.5f}, {0.5f, 0.3f, 0.5f}};

      QColor                                bgColor_                      {};
      QColor                                fogColor_                     {};
      ScaleBar                              scaleBar_                     {};
      std::vector<ClippingPlane>            clippingPlanes_               {};
      class Screenshot*                     screenshotTaker_              {nullptr};
      bool                                  saveScreenshotOnNewGeometry_  {false};
      bool                                  pulledSavedVisibility_        {false};
      QTimer                                resizeTimer_                  {};
      std::atomic<bool>                     pushingCameraState_           {false};

      //geometries
      Modules::Visualization::TextBuilder               textBuilder_        {};
      Graphics::Datatypes::GeometryHandle               scaleBarGeom_       {};
      std::vector<Graphics::Datatypes::GeometryHandle>  clippingPlaneGeoms_ {};
      std::vector<Graphics::Datatypes::WidgetHandle>    widgetHandles_      {};
      QAction*                                          lockRotation_       {nullptr};
      QAction*                                          lockPan_            {nullptr};
      QAction*                                          lockZoom_           {nullptr};
      QPushButton*                                      controlLock_        {nullptr};
      QPushButton*                                      autoViewButton_     {nullptr};
      QPushButton*                                      viewBarBtn_         {nullptr};

      std::vector<ViewSceneDialog*>                     viewScenesToUpdate  {};

      friend class ViewSceneControlsDock;

      std::unique_ptr<Core::GeometryIDGenerator> gid_;
      std::string                                       name_               {""};
    };

  } // namespace Gui
} // namespace SCIRun

#endif
