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

#ifndef INTERFACE_MODULES_VIEW_SCENE_VTK_CONTROLS_H
#define INTERFACE_MODULES_VIEW_SCENE_VTK_CONTROLS_H

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
#include "Interface/Modules/Render/ui_ViewSceneShortcuts.h"

#ifndef Q_MOC_RUN
#include <Core/Datatypes/DatatypeFwd.h>
#include <Modules/Render/ViewScene.h>
#include <Modules/Render/ViewSceneVtk.h>
#endif
#include <Interface/Modules/Render/share.h>

class QwtKnob;
class ctkColorPickerButton;
class QToolBar;
class ctkPopupWidget;
class QPushButton;

namespace SCIRun {
namespace Gui {
    class ViewSceneVtkDialog;

  class VisibleItemManagerVtk : public QObject
    {
      Q_OBJECT
     public:
      VisibleItemManagerVtk(QTreeWidget* itemList, Dataflow::Networks::ModuleStateHandle state);
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

    class SCISHARE ViewSceneVtkControlPopupWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit ViewSceneVtkControlPopupWidget(ViewSceneVtkDialog* parent);
        QAction* pinToggleAction() { return pinToggleAction_; }
        QAction* closeAction() { return closeAction_; }
    private Q_SLOTS:
        void showContextMenu(const QPoint& pos);
    private:
        QAction* pinToggleAction_{ nullptr };
        QAction* closeAction_{ nullptr };
    };

  class SCISHARE AutoRotateControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::AutoRotateSettings
    {
      Q_OBJECT

     public:
      explicit AutoRotateControlsVtk(ViewSceneVtkDialog* parent);
    };

    class SCISHARE ColorOptionsVtk : public ViewSceneVtkControlPopupWidget, public Ui::ColorOptions
    {
      Q_OBJECT

     public:
      explicit ColorOptionsVtk(ViewSceneVtkDialog* parent);
      void setSampleColor(const QColor& color);
    };

    class SCISHARE MaterialsControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::Materials
    {
      Q_OBJECT

     public:
      explicit MaterialsControlsVtk(ViewSceneVtkDialog* parent);
      void setMaterialValues(double ambient, double diffuse, double specular, double shine, double emission);
    };

    class SCISHARE ButtonStylesheetTogglerVtk
    {
     public:
      ButtonStylesheetTogglerVtk(QPushButton* toolbarButton, std::function<void()> whatToToggle);
      void updateToolbarButton(const QColor& color);

     protected:
      QPushButton* toolbarButton_{nullptr};
      std::function<bool()> linkedCheckable_;
      std::function<void()> whatToToggle_;
    };

  class SCISHARE LightButtonUpdaterVtk : public ButtonStylesheetTogglerVtk
    {
     public:
      explicit LightButtonUpdaterVtk(QPushButton* toolbarButton, std::function<void()> whatToToggle);
      QColor color() const;
      void setColor(const QColor& color);

     protected:
      ctkColorPickerButton* colorPickerButton_{nullptr};
      QColor lightColor_;
      void updateLightColor();
      virtual void lightColorUpdated() = 0;
    };

    class SCISHARE FogControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::Fog, public LightButtonUpdaterVtk
    {
      Q_OBJECT

     public:
      FogControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton);
      void setFogValues(bool fogVisible, bool objectsOnly, bool useBGColor, double fogStart, double fogEnd);
     Q_SIGNALS:
      void setFogTo(bool toggle);
      void lightColorUpdated() override;
     public Q_SLOTS:
      void toggleFog();
    };

    class SCISHARE ObjectSelectionControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::ObjectSelection
    {
      Q_OBJECT

     public:
      explicit ObjectSelectionControlsVtk(ViewSceneVtkDialog* parent);
      VisibleItemManagerVtk& visibleItems() { return *visibleItems_; }

     private:
      void setupObjectListWidget();
      std::unique_ptr<VisibleItemManagerVtk> visibleItems_;
    };

    class SCISHARE OrientationAxesControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::OrientationAxes, public ButtonStylesheetTogglerVtk
    {
      Q_OBJECT

     public:
      explicit OrientationAxesControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton);
      void toggleButton();
     public Q_SLOTS:
      /// Single entry point for flipping the orientation icon on/off: drives the
      /// group box, which propagates to the dialog and the toolbar button.
      void toggleOrientation();

     private:
      void setSliderDefaultPos();
      void setSliderCenterPos();
    };

    class SCISHARE ScreenshotControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::Screenshot
    {
      Q_OBJECT

     public:
      explicit ScreenshotControlsVtk(ViewSceneVtkDialog* parent);
      void setScreenshotDirectory(const QString& dir);

     private:
      void setSliderDefaultPos();
      void setSliderCenterPos();
    };

    struct SCISHARE ScaleBarDataVtk
    {
      bool visible;
      int fontSize;
      double length, height, multiplier, numTicks, lineWidth, lineColor;
      std::string unit;
      double projLength;
    };

    class SCISHARE ScaleBarControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::ScaleBar, public ButtonStylesheetTogglerVtk
    {
      Q_OBJECT

     public:
      ScaleBarControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton);
      void setScaleBarValues(const ScaleBarDataVtk& scale);

     private:
      static const QColor buttonOutlineColor;
    };

    class SCISHARE ClippingPlaneControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::ClippingPlanes, public ButtonStylesheetTogglerVtk
    {
        Q_OBJECT

    public:
        ClippingPlaneControlsVtk(ViewSceneVtkDialog* parent, QPushButton* toolbarButton);
        void updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal);
        void updatePlaneControlDisplay(double x, double y, double z, double d);
    public Q_SLOTS:
        /// Single entry point for flipping clipping on/off: drives the check box,
        /// which propagates to the dialog and the toolbar button.
        void toggleVisible();
    };

  class SCISHARE InputControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::Input
    {
      Q_OBJECT

     public:
      explicit InputControlsVtk(ViewSceneVtkDialog* parent);
      void updateZoomOptionVisibility();
    };

    class SCISHARE CameraLockControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::CameraLocks
    {
      Q_OBJECT

     public:
      explicit CameraLockControlsVtk(ViewSceneVtkDialog* parent);
     private Q_SLOTS:
      void updateViewSceneTree();
      void addGroup();
      void removeGroup();
      void viewSceneTreeClicked(QTreeWidgetItem* widgetItem, int column);
    };

    class SCISHARE DeveloperControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::Developer
    {
      Q_OBJECT

     public:
      explicit DeveloperControlsVtk(ViewSceneVtkDialog* parent);
    };

#ifndef WIN32
#define LightSliderTypeVtk QwtKnob
#else
#define LightSliderTypeVtk QSlider
#endif

    class SCISHARE LightControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::LightControls, public LightButtonUpdaterVtk
    {
      Q_OBJECT

     public:
      explicit LightControlsVtk(ViewSceneVtkDialog* parent, int lightNumber, QPushButton* toolbarButton);
      void setAdditionalLightState(int azimuth, int inclination, bool on);
      /// Single entry point for switching this light on/off: drives the check box,
      /// which propagates to the dialog and the toolbar button.
      void setLightOn(bool on);
      void toggleLightOn();
      bool isLightOn() const;

     private:
      int lightNumber_{-1};
      LightSliderTypeVtk* lightAzimuthSlider_{nullptr};
      LightSliderTypeVtk* lightInclinationSlider_{nullptr};

     private Q_SLOTS:
      void resetAngles();
     Q_SIGNALS:
      void lightColorUpdated() override;
    };

    class SCISHARE CompositeLightControlsVtk : public ViewSceneVtkControlPopupWidget
    {
      Q_OBJECT
     public:
      explicit CompositeLightControlsVtk(ViewSceneVtkDialog* parent, const std::vector<LightControlsVtk*>& secondaryLights);

     private:
      std::vector<LightControlsVtk*> lights_;
      QTabWidget* tabs_;
    };

    class SCISHARE ViewAxisChooserControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::ViewAxisChooser
    {
      Q_OBJECT
     public:
      explicit ViewAxisChooserControlsVtk(ViewSceneVtkDialog* parent);
      QString currentAxis() const;
     private Q_SLOTS:
      void viewAxisSelected(const QString& name);
    };

    class SCISHARE ViewSceneVtkToolBarController : public QObject
    {
      Q_OBJECT
     public:
      explicit ViewSceneVtkToolBarController(ViewSceneVtkDialog* dialog);
      void setDefaultProperties(QToolBar* toolbar, ctkPopupWidget* popup);
      void registerPopup(QToolBar* toolbar, ctkPopupWidget* popup);
      void registerDirectionButton(QToolBar* toolbar, QPushButton* button);
      void updateDelays();
      static constexpr const char* DirectionProperty = "dir";
      static constexpr const char* FlipProperty = "flip";

     private:
      ViewSceneVtkDialog* dialog_;
      void updatePopupProperties(QToolBar* toolbar, ctkPopupWidget* popup, bool flipped);
      std::map<QToolBar*, std::vector<ctkPopupWidget*>> toolBarPopups_;
    };
}
}

#endif
