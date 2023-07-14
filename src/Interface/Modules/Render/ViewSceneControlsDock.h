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

#ifndef INTERFACE_MODULES_VIEW_SCENE_CONTROLS_H
#define INTERFACE_MODULES_VIEW_SCENE_CONTROLS_H

#include "Interface/Modules/Render/ui_AutoRotateSettings.h"
#include "Interface/Modules/Render/ui_CameraLocks.h"
#include "Interface/Modules/Render/ui_ClippingPlanes.h"
#include "Interface/Modules/Render/ui_ColorOptions.h"
#include "Interface/Modules/Render/ui_DevControls.h"
#include "Interface/Modules/Render/ui_Fog.h"
#include "Interface/Modules/Render/ui_InputControls.h"
#include "Interface/Modules/Render/ui_LightControls.h"
#include "Interface/Modules/Render/ui_Materials.h"
#include "Interface/Modules/Render/ui_ObjectSelection.h"
#include "Interface/Modules/Render/ui_OrientationAxes.h"
#include "Interface/Modules/Render/ui_ScaleBar.h"
#include "Interface/Modules/Render/ui_Screenshot.h"
#include "Interface/Modules/Render/ui_ViewAxisChooser.h"
#include "Interface/Modules/Render/ui_ViewSceneControls.h"

#ifndef Q_MOC_RUN
#include <Core/Datatypes/DatatypeFwd.h>
#include <Modules/Render/ViewScene.h>
#endif
#include <Interface/Modules/Render/share.h>

class QwtKnob;
class ctkColorPickerButton;
class QToolBar;
class ctkPopupWidget;

namespace SCIRun {
namespace Gui {
  class ViewSceneDialog;

  class VisibleItemManager : public QObject
  {
    Q_OBJECT
   public:
    VisibleItemManager(QTreeWidget* itemList, Dataflow::Networks::ModuleStateHandle state);
    std::vector<QString> synchronize(const std::vector<Core::Datatypes::GeometryBaseHandle>& geomList, const Modules::Render::ShowFieldStatesMap& showFieldStates);
    bool isVisible(const QString& name) const;
    bool containsItem(const QString& name) const;
    void initializeSavedStateMap();
   public Q_SLOTS:
    void clear();
    void selectAllClicked();
    void deselectAllClicked();
    void updateVisible(QTreeWidgetItem* item, int column);
   Q_SIGNALS:
    void visibleItemChange();
    void meshComponentSelectionChange(const QString& moduleId, const QString& component, bool selected);
   private Q_SLOTS:
    void updateState();

   private:
    void addRenderItem(const QString& name);
    void updateCheckStates(const QString& name, const std::vector<bool>& checked);
    QTreeWidget* itemList_;
    Dataflow::Networks::ModuleStateHandle state_;
    std::map<QString, bool> topLevelItemMap_;
    std::map<QString, std::map<QString, bool>> secondLevelItemMap_;
  };

  class SCISHARE ViewSceneControlPopupWidget : public QWidget
  {
    Q_OBJECT

  public:
    explicit ViewSceneControlPopupWidget(ViewSceneDialog* parent);
    QAction* pinToggleAction() { return pinToggleAction_; }
    QAction* closeAction() { return closeAction_; }
  private Q_SLOTS:
    void showContextMenu(const QPoint& pos);
  private:
    QAction* pinToggleAction_{ nullptr };
    QAction* closeAction_{ nullptr };
  };

  class SCISHARE AutoRotateControls : public ViewSceneControlPopupWidget, public Ui::AutoRotateSettings
  {
    Q_OBJECT

   public:
    explicit AutoRotateControls(ViewSceneDialog* parent);
  };

  class SCISHARE ColorOptions : public ViewSceneControlPopupWidget, public Ui::ColorOptions
  {
    Q_OBJECT

   public:
    explicit ColorOptions(ViewSceneDialog* parent);
    void setSampleColor(const QColor& color);
  };

  class SCISHARE MaterialsControls : public ViewSceneControlPopupWidget, public Ui::Materials
  {
    Q_OBJECT

   public:
    explicit MaterialsControls(ViewSceneDialog* parent);
    void setMaterialValues(double ambient, double diffuse, double specular, double shine, double emission);
  };

  class SCISHARE ButtonStylesheetToggler
  {
   public:
    ButtonStylesheetToggler(QPushButton* toolbarButton, std::function<void()> whatToToggle);
    void updateToolbarButton(const QColor& color);

   protected:
    QPushButton* toolbarButton_{nullptr};
    std::function<bool()> linkedCheckable_;
    std::function<void()> whatToToggle_;
  };

  class SCISHARE LightButtonUpdater : public ButtonStylesheetToggler
  {
   public:
    explicit LightButtonUpdater(QPushButton* toolbarButton, std::function<void()> whatToToggle);
    QColor color() const;
    void setColor(const QColor& color);

   protected:
    ctkColorPickerButton* colorPickerButton_{nullptr};
    QColor lightColor_;
    void updateLightColor();
    virtual void lightColorUpdated() = 0;
  };

  class SCISHARE FogControls : public ViewSceneControlPopupWidget, public Ui::Fog, public LightButtonUpdater
  {
    Q_OBJECT

   public:
    FogControls(ViewSceneDialog* parent, QPushButton* toolbarButton);
    void setFogValues(bool fogVisible, bool objectsOnly, bool useBGColor, double fogStart, double fogEnd);
   Q_SIGNALS:
    void setFogTo(bool toggle);
    void lightColorUpdated() override;
   public Q_SLOTS:
    void toggleFog();
  };

  class SCISHARE ObjectSelectionControls : public ViewSceneControlPopupWidget, public Ui::ObjectSelection
  {
    Q_OBJECT

   public:
    explicit ObjectSelectionControls(ViewSceneDialog* parent);
    VisibleItemManager& visibleItems() { return *visibleItems_; }

   private:
    void setupObjectListWidget();
    std::unique_ptr<VisibleItemManager> visibleItems_;
  };

  class SCISHARE OrientationAxesControls : public ViewSceneControlPopupWidget, public Ui::OrientationAxes, public ButtonStylesheetToggler
  {
    Q_OBJECT

   public:
    explicit OrientationAxesControls(ViewSceneDialog* parent, QPushButton* toolbarButton);
    void toggleButton();

   private:
    void setSliderDefaultPos();
    void setSliderCenterPos();
  };

  class SCISHARE ScreenshotControls : public ViewSceneControlPopupWidget, public Ui::Screenshot
  {
    Q_OBJECT

   public:
    explicit ScreenshotControls(ViewSceneDialog* parent);
    void setScreenshotDirectory(const QString& dir);

   private:
    void setSliderDefaultPos();
    void setSliderCenterPos();
  };

  struct SCISHARE ScaleBarData
  {
    bool visible;
    int fontSize;
    double length, height, multiplier, numTicks, lineWidth, lineColor;
    std::string unit;
    double projLength;
  };

  class SCISHARE ScaleBarControls : public ViewSceneControlPopupWidget, public Ui::ScaleBar, public ButtonStylesheetToggler
  {
    Q_OBJECT

   public:
    ScaleBarControls(ViewSceneDialog* parent, QPushButton* toolbarButton);
    void setScaleBarValues(const ScaleBarData& scale);

   private:
    static const QColor buttonOutlineColor;
  };

  class SCISHARE ClippingPlaneControls : public ViewSceneControlPopupWidget, public Ui::ClippingPlanes, public ButtonStylesheetToggler
  {
    Q_OBJECT

   public:
    ClippingPlaneControls(ViewSceneDialog* parent, QPushButton* toolbarButton);
    void updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal);
    void updatePlaneControlDisplay(double x, double y, double z, double d);
  };

  class SCISHARE InputControls : public ViewSceneControlPopupWidget, public Ui::Input
  {
    Q_OBJECT

   public:
    explicit InputControls(ViewSceneDialog* parent);
    void updateZoomOptionVisibility();
  };

  class SCISHARE CameraLockControls : public ViewSceneControlPopupWidget, public Ui::CameraLocks
  {
    Q_OBJECT

   public:
    explicit CameraLockControls(ViewSceneDialog* parent);
   private Q_SLOTS:
    void updateViewSceneTree();
    void addGroup();
    void removeGroup();
    void viewSceneTreeClicked(QTreeWidgetItem* widgetItem, int column);
  };

  class SCISHARE DeveloperControls : public ViewSceneControlPopupWidget, public Ui::Developer
  {
    Q_OBJECT

   public:
    explicit DeveloperControls(ViewSceneDialog* parent);
  };

#ifndef WIN32
#define LightSliderType QwtKnob
#else
#define LightSliderType QSlider
#endif

  class SCISHARE LightControls : public ViewSceneControlPopupWidget, public Ui::LightControls, public LightButtonUpdater
  {
    Q_OBJECT

   public:
    explicit LightControls(ViewSceneDialog* parent, int lightNumber, QPushButton* toolbarButton);
    void setAdditionalLightState(int azimuth, int inclination, bool on);

   private:
    int lightNumber_{-1};
    LightSliderType* lightAzimuthSlider_{nullptr};
    LightSliderType* lightInclinationSlider_{nullptr};

   private Q_SLOTS:
    void resetAngles();
   Q_SIGNALS:
    void lightColorUpdated() override;
  };

  class SCISHARE CompositeLightControls : public ViewSceneControlPopupWidget
  {
    Q_OBJECT
  public:
    explicit CompositeLightControls(ViewSceneDialog* parent, const std::vector<LightControls*>& secondaryLights);
  private:
    std::vector<LightControls*> lights_;
    QTabWidget* tabs_;
  };

  class SCISHARE ViewAxisChooserControls : public ViewSceneControlPopupWidget, public Ui::ViewAxisChooser
  {
    Q_OBJECT
   public:
    explicit ViewAxisChooserControls(ViewSceneDialog* parent);
    QString currentAxis() const;
   private Q_SLOTS:
    void viewAxisSelected(const QString& name);
  };

  class SCISHARE ViewSceneToolBarController : public QObject
  {
    Q_OBJECT
   public:
     explicit ViewSceneToolBarController(ViewSceneDialog* dialog);
     void setDefaultProperties(QToolBar* toolbar, ctkPopupWidget* popup);
     void registerPopup(QToolBar* toolbar, ctkPopupWidget* popup);
     void registerDirectionButton(QToolBar* toolbar, QPushButton* button);
     void updateDelays();
     static constexpr const char* DirectionProperty = "dir";
     static constexpr const char* FlipProperty = "flip";
   private:
     ViewSceneDialog* dialog_;
     void updatePopupProperties(QToolBar* toolbar, ctkPopupWidget* popup, bool flipped);
     std::map<QToolBar*, std::vector<ctkPopupWidget*>> toolBarPopups_;
   };
}
}

#endif
