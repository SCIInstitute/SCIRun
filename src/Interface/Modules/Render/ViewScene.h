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
#include <Core/Datatypes/Feedback.h>

//TODO: needs to inherit from ModuleWidget somehow
class QToolBar;
class QStandardItemModel;
class QStandardItem;

namespace SCIRun {
  namespace Gui {

    class GLWidget;
    class ScopedWidgetColorChanger;
    class ViewSceneDialogImpl;

    class SCISHARE ViewSceneDialog : public ModuleDialogGeneric, public Ui::ViewScene
    {
    Q_OBJECT;

    public:
      ViewSceneDialog(const std::string& name, Dataflow::Networks::ModuleStateHandle state,
        QWidget* parent = nullptr);
      ~ViewSceneDialog() override;

      std::string toString(std::string prefix) const;
      void adjustToolbar() override;

      static ViewSceneManager viewSceneManager;
      void inputMouseDownHelper(float x, float y);
      void inputMouseMoveHelper(MouseButton btn, float x, float y);
      void inputMouseUpHelper();
      void inputMouseWheelHelper(int32_t delta);
      void setViewScenesToUpdate(const std::unordered_set<ViewSceneDialog*>& scenes);
      std::string getName() const;
      void autoSaveScreenshot();
      void setFloatingState(bool isFloating);
      void vsLog(const QString& msg) const;

      void postMoveEventCallback(const QPoint& p) override;

    Q_SIGNALS:
      void newGeometryValueForwarder();
      void cameraRotationChangeForwarder();
      void cameraLookAtChangeForwarder();
      void cameraDistanceChangeForwarder();
      void lockMutexForwarder();
      void mousePressSignalForGeometryObjectFeedback(int x, int y, const std::string& selName);

    protected Q_SLOTS:
      void printToString() const {std::cout << toString("");}
      void sendBugReport();

      //---------------- New Geometry --------------------------------------------------------------
      void updateModifiedGeometriesAndSendScreenShot();

      void sendGeometryFeedbackToState(int x, int y, const std::string& selName);
      void frameFinished();
      void lockMutex();

      void runDelayedGC();

      //---------------- Input ---------------------------------------------------------------------
      void resizingDone();

      //---------------- Camera --------------------------------------------------------------------
      void autoViewClicked();
      void menuMouseControlChanged(int index);
      void invertZoomClicked(bool value);
      void adjustZoomSpeed(int value);
      void lockRotationToggled();
      void lockPanningToggled();
      void lockZoomToggled();
      void lockAllTriggered();
      void unlockAllTriggered();
      void toggleLockColor(bool locked);
      void setAutoRotateSpeed(double speed);
      void autoRotateRight();
      void autoRotateLeft();
      void autoRotateUp();
      void autoRotateDown();
      void toggleAutoRotate();
      void pullCameraRotation();
      void pullCameraLookAt();
      void pullCameraDistance();
      void snapToViewAxis();

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

      //---------------- Orientation Glyph ----------------------------------------------------------
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
      void toggleLight(int index, bool value);
      void setLightAzimuth(int index, int value);
      void setLightInclination(int index, int value);

      //---------------- Material Settings ---------------------------------------------------------
      void setAmbientValue(double value);
      void setDiffuseValue(double value);
      void setSpecularValue(double value);
      void setShininessValue(double value);

      //---------------- Fog Tools -----------------------------------------------------------------
      void setFogOn(bool value);
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
      void quickScreenshot(bool prompt);
      void quickScreenshotClicked() { quickScreenshot(true); }
      void saveNewGeometryChanged(int state);


    protected:
      //---------------- Initialization ------------------------------------------------------------
      void pullSpecial() override;

      void newGeometryValue(bool forceAllObjectsToUpdate, bool clippingPlanesUpdated);
      void updateAllGeometries();
      void updateModifiedGeometries();
      void unblockExecution();

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
      void contextMenuEvent(QContextMenuEvent*) override {}


    private:
      //---------------- Initialization ------------------------------------------------------------
      void addToolBar();
      void setupScaleBar();
      void setInitialLightValues();
      void setupMaterials();
      void addAutoViewButton();
      void addScreenshotButton();
      void addQuickScreenshotButton();
      void addViewBarButton();
      void addControlLockButton();
      void addAutoRotateButton();
      void addColorOptionsButton();
      void addLightOptionsComboBox();
      void addFogOptionsButton();
      void addMaterialOptionsButton();
      void addOrientationAxesButton();
      void addScaleBarButton();
      void addClippingPlaneButton();
      void addInputControlButton();
      void addCameraLocksButton();
      void addDeveloperControlButton();
      void addToolbarButton(QWidget* w, int which, QWidget* widgetToPopup = nullptr);
      void addConfigurationButton();
      void addObjectSelectionButton();
      void addLightButtons();
      QColor checkColorSetting(const std::string& rgb, const QColor& defaultColor);
      void pullCameraState();
      void pushCameraDistance();
      void pushCameraLookAt();
      void pushCameraRotation();
      void pushCameraState();
      bool clickedInViewer(QMouseEvent* e) const;
      void initializeAxes();
      void initializeVisibleObjects();

      //---------------- Widgets -------------------------------------------------------------------
      bool needToWaitForWidgetSelection();
      bool canSelectWidget();
      bool tryWidgetSelection(int x, int y, MouseButton button);
      void selectObject(const int x, const int y, MouseButton button);
      Modules::Render::ViewScene::GeomListPtr getGeomData();
      bool checkForSelectedWidget(Graphics::Datatypes::WidgetHandle widget);
      void restoreObjColor();
      void backupColorValues(Graphics::Datatypes::WidgetHandle widget);
      void updateCursor();

      //---------------- Clipping Planes -----------------------------------------------------------
      void updateClippingPlaneDisplay();
      void buildGeomClippingPlanes();
      void initializeClippingPlaneDisplay();
      void doClippingPlanes();
      void buildGeometryClippingPlane(int index, bool reverseNormal, const glm::vec4& plane, const Core::Geometry::BBox& bbox);

      //---------------- Scale Bar -----------------------------------------------------------------
      void updateScaleBarLength();
      Graphics::Datatypes::GeometryHandle buildGeometryScaleBar();

      //---------------- Materials -----------------------------------------------------------------
      void setMaterialFactor(Render::MatFactor factor, double value);

      //---------------- Fog -----------------------------------------------------------------------
      void setFog(Render::FogFactor factor, double value);
      void setFogColor(const glm::vec4 &color);

      //---------------- Misc. ---------------------------------------------------------------------
      void takeScreenshot();
      void sendScreenshotDownstreamForTesting();

      std::unique_ptr<ViewSceneDialogImpl> impl_;

      friend class ViewSceneControlsDock;
      friend class AutoRotateControls;
      friend class ColorOptions;
      friend class FogControls;
      friend class MaterialsControls;
      friend class ObjectSelectionControls;
      friend class OrientationAxesControls;
      friend class ScaleBarControls;
      friend class LightControls;
      friend class ClippingPlaneControls;
    };

    MouseButton getSpireButton(QMouseEvent* event);

  } // namespace Gui
} // namespace SCIRun

#endif
