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
#include "Interface/Modules/Render/ViewScene.h"

#ifndef Q_MOC_RUN
#include <Core/Datatypes/DatatypeFwd.h>
#include <Modules/Render/ViewScene.h>
#include <boost/atomic.hpp>
#endif
#include <QGraphicsView>
#include <Interface/Modules/Render/share.h>

namespace SCIRun {
  namespace Gui {
    class ViewSceneDialog;

    class VisibleItemManager : public QObject
    {
      Q_OBJECT
    public:
      explicit VisibleItemManager(QTreeWidget* itemList) : itemList_(itemList) {}
      std::vector<QString> synchronize(const std::vector<Core::Datatypes::GeometryBaseHandle>& geomList,
        const Modules::Render::ShowFieldStatesMap& showFieldStates);
      bool isVisible(const QString& name) const;
      bool containsItem(const QString& name) const;
    public Q_SLOTS:
      void clear();
    Q_SIGNALS:
      void visibleItemChange();
      void meshComponentSelectionChange(const QString& moduleId, const QString& component, bool selected);
    private Q_SLOTS:
      void updateVisible(QTreeWidgetItem* item, int column);
      void selectAllClicked();
      void deselectAllClicked();
    private:
      void addRenderItem(const QString& name);
      void updateCheckStates(const QString& name, std::vector<bool> checked);
      QTreeWidget* itemList_;
    };

    class SCISHARE ViewSceneControlsDock : public QDockWidget, public Ui::ViewSceneControls
    {
      Q_OBJECT

    public:
      ViewSceneControlsDock(const QString& name, ViewSceneDialog* parent);
      void setSampleColor(const QColor& color);
      void setFogColorLabel(const QColor& color);
      void setLabelColor(QLabel* label, const QColor& color);
      void setMaterialTabValues(double ambient, double diffuse, double specular, double shine, double emission,
        bool fogVisible, bool objectsOnly, bool useBGColor, double fogStart, double fogEnd);
      void setScaleBarValues(bool visible, int fontSize, double length, double height, double multiplier,
        double numTicks, double lineWidth, const QString& unit);
      void setRenderTabValues(bool lighting, bool bbox, bool useClip, bool backCull, bool displayList, bool stereo,
        double stereoFusion, double polygonOffset, double textOffset, int fov);
      void updateZoomOptionVisibility();
      void updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal);
      void updatePlaneControlDisplay(double x, double y, double z, double d);
      QColor getLightColor(int index) const;

      VisibleItemManager& visibleItems() { return *visibleItems_; }

    private:
      void setupObjectListWidget();
      QColor lightColors[4];

      std::unique_ptr<VisibleItemManager> visibleItems_;

    Q_SIGNALS:
      void updateLightColor(const int index);

    private Q_SLOTS:
      void selectLightColor(int index);
      void selectLight0Color() {selectLightColor(0);}
      void selectLight1Color() {selectLightColor(1);}
      void selectLight2Color() {selectLightColor(2);}
      void selectLight3Color() {selectLightColor(3);}
      void setSliderDefaultPos();
      void setSliderCenterPos();
      void updateViewSceneTree();
      void addGroup();
      void removeGroup();
      void viewSceneTreeClicked(QTreeWidgetItem* widgetItem, int column);

    };
  }
}

#endif //INTERFACE_MODULES_VIEW_SCENE_CONTROLS_H
