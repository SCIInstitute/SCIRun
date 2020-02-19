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


#ifndef INTERFACE_MODULES_VIEW_OSPRAY_SCENE_CONTROLS_H
#define INTERFACE_MODULES_VIEW_OSPRAY_SCENE_CONTROLS_H

#include "Interface/Modules/Render/ui_ViewOspraySceneConfig.h"

#ifndef Q_MOC_RUN
#include <Core/Datatypes/DatatypeFwd.h>
//#include <Modules/Render/ViewScene.h>
#include <boost/atomic.hpp>
#endif
#include <QDialog>
#include <Interface/Modules/Render/share.h>

namespace SCIRun {
  namespace Gui {

    class OsprayViewerDialog;

    class SCISHARE ViewOspraySceneConfigDialog : public QDialog, public Ui::ViewOspraySceneConfig
    {
      Q_OBJECT

    public:
      ViewOspraySceneConfigDialog(const QString& name, OsprayViewerDialog* parent);
      // void setSampleColor(const QColor& color);
      // void setFogColorLabel(const QColor& color);
      // void setMaterialTabValues(double ambient, double diffuse, double specular, double shine, double emission,
      //   bool fogVisible, bool objectsOnly, bool useBGColor, double fogStart, double fogEnd);
      // void setScaleBarValues(bool visible, int fontSize, double length, double height, double multiplier,
      //   double numTicks, double lineWidth, const QString& unit);
      // void setRenderTabValues(bool lighting, bool bbox, bool useClip, bool backCull, bool displayList, bool stereo,
      //   double stereoFusion, double polygonOffset, double textOffset, int fov);
      // void updateZoomOptionVisibility();
      // void updatePlaneSettingsDisplay(bool visible, bool showPlane, bool reverseNormal);
      // void updatePlaneControlDisplay(double x, double y, double z, double d);
      // QPointF getLightPosition(int index) const;
      // QColor getLightColor(int index) const;
      //
      // VisibleItemManager& visibleItems() { return *visibleItems_; }

    private:
      // void setupObjectListWidget();
      // void setupLightControlCircle(QFrame* frame, int index, const boost::atomic<bool>& pulling, bool moveable);
      //
      // std::vector<LightControlCircle*> lightControls_;
      // std::unique_ptr<VisibleItemManager> visibleItems_;
    };
  }
}

#endif //INTERFACE_MODULES_VIEW_SCENE_CONTROLS_H
