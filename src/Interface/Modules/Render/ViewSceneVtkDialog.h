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
  ViewSceneVtkDialog(const std::string& name,
    Dataflow::Networks::ModuleStateHandle state,
    QWidget* parent = nullptr);
  ~ViewSceneVtkDialog();
  void adjustToolbar(double factor) override;
  Qt::ToolBarArea whereIs(QToolBar* toolbar) const;

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

private Q_SLOTS:
  void newGeometryValue();
  void autoViewClicked();
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

      //---------------- Clipping Planes -----------------------------------------------------------
  void updateClippingPlaneDisplay();
  //void buildGeomClippingPlanes();
  void initializeClippingPlaneDisplay();
  void doClippingPlanes();

  void setClippingPlaneIndex(int index);
  void setClippingPlaneVisible(bool value);
  void setClippingPlaneFrameOn(bool value);
  void reverseClippingPlaneNormal(bool value);
  void setClippingPlaneX(int index);
  void setClippingPlaneY(int index);
  void setClippingPlaneZ(int index);
  void setClippingPlaneD(int index);

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
  void toggleLockColor(bool locked);
  void mousePositionToScreenSpace(int xIn, int yIn, float& xOut, float& yOut);
  MouseButton getRenderButton(QMouseEvent* event);
  void setupPopupWidget(QPushButton* button, ViewSceneVtkControlPopupWidget* underlyingWidget, QToolBar* toolbar);

  float getFloat(const Core::Algorithms::Name& name) const;

  Render::VtkQWidget* viewer_ {nullptr};

  Render::VtkRenderer* renderer_{nullptr};

  QStatusBar* statusBar_ {nullptr};
  QToolBar* toolBar1_ {nullptr};
  QToolBar* toolBar2_{nullptr};
  ViewSceneVtkToolBarController* toolBarController_{nullptr};

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

  friend class ClippingPlaneControlsVtk;
};
}}

#endif
