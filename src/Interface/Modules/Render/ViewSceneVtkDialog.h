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


#ifndef INTERFACE_MODULES_RENDER_VIEWSCENEVTKDIALOG_H
#define INTERFACE_MODULES_RENDER_VIEWSCENEVTKDIALOG_H

#include "Interface/Modules/Render/ui_ViewSceneVtk.h"
/// \todo Make this definition specific to windows.
#define NOMINMAX
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Render/Vtk/VtkRenderer.h>
#include <Interface/Modules/Render/Vtk/VtkQWidget.h>
#include <Core/Datatypes/Geometry.h>
#include <Core/Datatypes/Feedback.h>
#include <Interface/Modules/Render/ViewSceneVtkManager.h>
#include <Interface/Modules/Render/share.h>

#include <Interface/Modules/Render/ViewSceneVtkControlsDock.h>
#include <Interface/Modules/Render/ES/RendererCollaborators.h>

namespace SCIRun {

namespace Gui {

//class ViewSceneVtkConfigDialog;

class SCISHARE ViewSceneVtkDialog : public ModuleDialogGeneric, public Ui::ViewSceneVtk
{
  Q_OBJECT;

public:
  // -------- Keyboard shortcut registry ----------------------------------------
  struct ShortcutDef
  {
    enum class Id : int
    {
      AxisViews,
      Autoview,
      AutoviewNoScale,
      SnapToAxis,
      CopyView,
      SetHome,
      GotoHome,
      ToggleAxes,
      BoundingBox,
      ToggleClipping,
      ToggleFog,
      FlatShading,
      OpenHelp,
      ViewLocking,
      ToggleLighting,
      OrientationIcon,
      Orthographic,
      Stereo,
      Backculling,
      Wireframe,
      NUM_SHORTCUTS
    };
    using Action = std::function<void(ViewSceneVtkDialog*)>;

    Id id;
    Qt::Key key;
    Qt::KeyboardModifiers modifiers{Qt::NoModifier};
    const char* actionName;
    const char* shortcutDisplay;  // human-readable, e.g. "Ctrl+H" or "1-8"
    const char* description;
    Action action{nullptr};  // null = not yet implemented / shown grayed

    bool isImplemented() const { return static_cast<bool>(action); }
  };

  static constexpr auto numShortcuts = static_cast<std::size_t>(ShortcutDef::Id::NUM_SHORTCUTS);
  using ShortcutTable = std::array<ShortcutDef, numShortcuts>;

  ViewSceneVtkDialog(const std::string& name,
    Dataflow::Networks::ModuleStateHandle state,
    QWidget* parent = nullptr);
  ~ViewSceneVtkDialog();

  std::string toString(std::string prefix) const;
  void adjustToolbar(double factor) override;

  void vsLog(const QString& msg) const;
  Qt::ToolBarArea whereIs(QToolBar* toolbar) const;

  void setViewScenesToUpdate(const std::unordered_set<ViewSceneVtkDialog*>& scenes);
  std::string getName() const;

  static ViewSceneVtkManager viewSceneManager;

Q_SIGNALS:
  void newGeometryValueForwarder();
  void closeAllNonPinnedPopups();
  void fullScreenChanged();

protected:
  void pullSpecial() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void contextMenuEvent(QContextMenuEvent*) override {}

public Q_SLOTS:
  void printToString() const { std::cout << toString(""); }
  void sendBugReport();
  void adjustZoomSpeed(int value);
  void saveNewGeometryChanged(int state);
  void invertZoomClicked(bool value);
  void menuMouseControlChanged(int index);
  void adaptToFullScreenView(bool fullScreen) override;

 protected Q_SLOTS:
  void newGeometryValue();
  void autoRotateClicked();
  void screenshotClicked();
  void nextTimestepClicked();
  void playTimestepsClicked();
  void configButtonClicked();
  void setHeight(int h);
  void setWidth(int w);
  void setViewportCamera();
  void setLightColor();
  void setBGColor();
  void setCameraWidgets();

      //---------------- Input ---------------------------------------------------------------------
  void resizingDone();

  //---------------- Camera --------------------------------------------------------------------
  void autoViewClicked();
  void autoViewNoScaleClicked();
  void lockRotationToggled();
  void lockPanningToggled();
  void lockZoomToggled();
  void lockAllTriggered();
  void unlockAllTriggered();
  void toggleAllLocks();
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
  void setScaleBarLineColor(double value);
  void setScaleBar();

      //---------------- Lights --------------------------------------------------------------------
  void setLightColor(int index);
  void toggleLight(int index, bool value);
  void setLightAzimuth(int index, int value);
  void setLightInclination(int index, int value);
  void toggleAllLights();

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

  //---------------- Help ----------------------------------------------------------------------
  void showShortcutsDialog();

  //---------------- Misc. ---------------------------------------------------------------------
  void assignBackgroundColor();
  void setTransparencySortTypeContinuous(bool index);
  void setTransparencySortTypeUpdate(bool index);
  void setTransparencySortTypeLists(bool index);
  void screenshotSaveAs();
  void screenshotSaveAsClicked() { screenshotSaveAs(); };
  void quickScreenshot();
  void quickScreenshotClicked() { quickScreenshot(); }
  void setScreenshotDirectory();
  void setToolBarPositions();

 private:
  void addToolBar();
  void addConfigurationButton();
  void addConfigurationDialog();
  void addAutoViewButton();
  void addViewBarButton();
  void addAutoRotateButton();
  void addTimestepButtons();
  void addScreenshotButton();
  void addControlLockButton();
  void addClippingPlaneButton();
  void addToolbarButton(QPushButton* button);
  void addToolbarButton(QWidget* w, Qt::ToolBarArea area, ViewSceneVtkControlPopupWidget* widgetToPopup = nullptr);
  void mousePositionToScreenSpace(int xIn, int yIn, float& xOut, float& yOut);
  MouseButton getRenderButton(QMouseEvent* event);
  void setupPopupWidget(QPushButton* button, ViewSceneVtkControlPopupWidget* underlyingWidget, QToolBar* toolbar);

  float getFloat(const Core::Algorithms::Name& name) const;

  void setAxisView(int n);  // n=1..6 → +X,-X,+Y,-Y,+Z,-Z
  void setClosestAxisView();  // snap to nearest cardinal axis

  //clipping planes
  void updateClippingPlaneDisplay();
  void initializeClippingPlaneDisplay();
  void doClippingPlanes();

  void setMaterialFactor(Render::MatFactor factor, double value);

  void setFog(Render::FogFactor factor, double value);
  void setFogColor(const glm::vec4& color);

  void saveScreenshot(QString directory, bool notify);

  static const ShortcutTable& shortcutTable();

  std::string name_;

  Render::VtkQWidget* viewer_ {nullptr};

  Render::VtkRenderer* renderer_{nullptr};

  QStatusBar* statusBar_ {nullptr};
  QToolBar* toolBar1_ {nullptr};
  QToolBar* toolBar2_{nullptr};
  ViewSceneVtkToolBarController* toolBarController_{nullptr};
  QMainWindow* toolbarHolder_{nullptr};
  bool pulledSavedVisibility_{false};
  QLabel* statusLabel_{nullptr};
  QDialog* shortcutsDialog_{nullptr};
  QTableWidget* shortcutsTable_{nullptr};
  std::optional<QPoint> shortcutsDialogPos_{};

  //ViewOspraySceneConfigDialog* configDialog_;
  QAction* lockRotation_;
  QAction* lockPan_;
  QAction* lockZoom_;
  QPushButton* controlLock_;
  QPushButton* autoViewButton_;
  QPushButton* autoRotateButton_;
  QPushButton* playTimestepsButton_;

  ClippingPlaneControlsVtk* clippingPlaneControls_{nullptr};
  Render::ClippingPlaneManagerPtr clippingPlaneManager_;
  InputControlsVtk* inputControls_{nullptr};

  ColorOptionsVtk* colorOptions_{nullptr};
  FogControlsVtk* fogControls_{nullptr};

  QPushButton* toolBar1Position_{nullptr};
  QPushButton* toolBar2Position_{nullptr};

  std::vector<ViewSceneVtkDialog*> viewScenesToUpdate{};

  bool saveScreenshotOnNewGeometry_{false};

  std::array<bool, 3> lockStateBeforeAllOff_{{true, true, true}};

  ScaleBarDataVtk scaleBar_{};

  static constexpr int NUM_LIGHTS = 4;
  std::array<LightControlsVtk*, NUM_LIGHTS> lightControls_;
  std::array<bool, NUM_LIGHTS> lightStateBeforeAllOff_{{true, false, false, false}};

  QColor bgColor_{};

  OrientationAxesControlsVtk* orientationAxesControls_{nullptr};

  bool isFullScreen_{false};

  friend class ViewSceneControlsDockVtk;
  friend class AutoRotateControlsVtk;
  friend class ColorOptionsVtk;
  friend class FogControlsVtk;
  friend class MaterialsControlsVtk;
  friend class ObjectSelectionControlsVtk;
  friend class OrientationAxesControlsVtk;
  friend class ScreenshotControlsVtk;
  friend class ScaleBarControlsVtk;
  friend class LightControlsVtk;
  friend class ClippingPlaneControlsVtk;
};
}}

#endif
