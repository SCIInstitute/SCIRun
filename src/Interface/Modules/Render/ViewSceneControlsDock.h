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

#include "Interface/Modules/Render/ui_ViewSceneControls.h"
#include "Interface/Modules/Render/ui_AutoRotateSettings.h"
#include "Interface/Modules/Render/ui_ColorOptions.h"
#include "Interface/Modules/Render/ui_Materials.h"
#include "Interface/Modules/Render/ui_Fog.h"
#include "Interface/Modules/Render/ui_ObjectSelection.h"
#include "Interface/Modules/Render/ui_OrientationAxes.h"
#include "Interface/Modules/Render/ui_ScaleBar.h"
#include "Interface/Modules/Render/ui_ClippingPlanes.h"
#include "Interface/Modules/Render/ui_InputControls.h"
#include "Interface/Modules/Render/ui_CameraLocks.h"
#include "Interface/Modules/Render/ui_DevControls.h"
#include "Interface/Modules/Render/ui_LightControls.h"
#include "Interface/Modules/Render/ui_ViewAxisChooser.h"

#ifndef Q_MOC_RUN
#include <Core/Datatypes/DatatypeFwd.h>
#include <Modules/Render/ViewScene.h>
#endif
#include <Interface/Modules/Render/share.h>

class QwtKnob;
class ctkColorPickerButton;

namespace SCIRun {
  namespace Gui {
    class ViewSceneDialog;

    class VisibleItemManager : public QObject
    {
      Q_OBJECT
    public:
      VisibleItemManager(QTreeWidget* itemList, Dataflow::Networks::ModuleStateHandle state);
      std::vector<QString> synchronize(const std::vector<Core::Datatypes::GeometryBaseHandle>& geomList,
        const Modules::Render::ShowFieldStatesMap& showFieldStates);
      bool isVisible(const QString& name) const;
      bool containsItem(const QString& name) const;
      void initializeSavedStateMap();
    public Q_SLOTS:
      void clear();
      void selectAllClicked();
      void deselectAllClicked();
    Q_SIGNALS:
      void visibleItemChange();
      void meshComponentSelectionChange(const QString& moduleId, const QString& component, bool selected);
    private Q_SLOTS:
      void updateVisible(QTreeWidgetItem* item, int column);
      void updateState();
    private:
      void addRenderItem(const QString& name);
      void updateCheckStates(const QString& name, const std::vector<bool>& checked);
      QTreeWidget* itemList_;
      Dataflow::Networks::ModuleStateHandle state_;
      std::map<QString, bool> topLevelItemMap_;
      std::map<QString, std::map<QString, bool>> secondLevelItemMap_;
    };

    class SCISHARE AutoRotateControls : public QWidget, public Ui::AutoRotateSettings
    {
      Q_OBJECT

    public:
      explicit AutoRotateControls(ViewSceneDialog* parent);
    };

    class SCISHARE ColorOptions : public QWidget, public Ui::ColorOptions
    {
      Q_OBJECT

    public:
      explicit ColorOptions(ViewSceneDialog* parent);
      void setSampleColor(const QColor& color);
    };

    class SCISHARE MaterialsControls : public QWidget, public Ui::Materials
    {
      Q_OBJECT

    public:
      explicit MaterialsControls(ViewSceneDialog* parent);
      void setMaterialValues(double ambient, double diffuse, double specular, double shine, double emission);
    };

    class SCISHARE FogControls : public QWidget, public Ui::Fog
    {
      Q_OBJECT

    public:
      explicit FogControls(ViewSceneDialog* parent);
      void setFogColorLabel(const QColor& color);
      void setFogValues(bool fogVisible, bool objectsOnly, bool useBGColor, double fogStart, double fogEnd);
    };

    class SCISHARE ObjectSelectionControls : public QWidget, public Ui::ObjectSelection
    {
      Q_OBJECT

    public:
      explicit ObjectSelectionControls(ViewSceneDialog* parent);
      VisibleItemManager& visibleItems() { return *visibleItems_; }
    private:
      void setupObjectListWidget();
      std::unique_ptr<VisibleItemManager> visibleItems_;
    };

    class SCISHARE OrientationAxesControls : public QWidget, public Ui::OrientationAxes
    {
      Q_OBJECT

    public:
      explicit OrientationAxesControls(ViewSceneDialog* parent);
    private:
      void setSliderDefaultPos();
      void setSliderCenterPos();
    };

    struct SCISHARE ScaleBarData
    {
      bool visible;
      int fontSize;
      double length, height, multiplier, numTicks, lineWidth;
      std::string unit;
      double projLength;
    };

    class SCISHARE ScaleBarControls : public QWidget, public Ui::ScaleBar
    {
      Q_OBJECT

    public:
      explicit ScaleBarControls(ViewSceneDialog* parent);
      void setScaleBarValues(const ScaleBarData& scale);
    };

    class SCISHARE ClippingPlaneControls : public QWidget, public Ui::ClippingPlanes
    {
      Q_OBJECT

    public:
      explicit ClippingPlaneControls(ViewSceneDialog* parent);
      void updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal);
      void updatePlaneControlDisplay(double x, double y, double z, double d);
    };

    class SCISHARE InputControls : public QWidget, public Ui::Input
    {
      Q_OBJECT

    public:
      explicit InputControls(ViewSceneDialog* parent);
      void updateZoomOptionVisibility();
    };

    class SCISHARE CameraLockControls : public QWidget, public Ui::CameraLocks
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

    class SCISHARE DeveloperControls : public QWidget, public Ui::Developer
    {
      Q_OBJECT

    public:
      explicit DeveloperControls(ViewSceneDialog* parent);
    };

    class SCISHARE LightControls : public QWidget, public Ui::LightControls
    {
      Q_OBJECT

    public:
      explicit LightControls(ViewSceneDialog* parent, int lightNumber, QPushButton* toolbarButton);
      QColor getLightColor() const;
      void setColor(const QColor& color);
      void setState(int azimuth, int inclination, bool on);

    private:
      int lightNumber_ {-1};
      QColor lightColor_;
      QwtKnob* lightAzimuthSlider_{nullptr};
      QwtKnob* lightInclinationSlider_{ nullptr };
      ctkColorPickerButton* colorPickerButton_{nullptr};
      QPushButton* toolbarButton_{nullptr};

    Q_SIGNALS:
      void lightColorUpdated();

    private Q_SLOTS:
      void updateLightColor();
    };

    class SCISHARE ViewAxisChooserControls : public QWidget, public Ui::ViewAxisChooser
    {
      Q_OBJECT
    public:
      explicit ViewAxisChooserControls(ViewSceneDialog* parent);
      QString currentAxis() const;
    private Q_SLOTS:
      void viewAxisSelected(const QString& name);
    };
  }
}

#endif
