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


#ifndef INTERFACE_MODULES_RENDER_OSPRAYVIEWERDIALOG_H
#define INTERFACE_MODULES_RENDER_OSPRAYVIEWERDIALOG_H

#include "Interface/Modules/Render/ui_OsprayViewer.h"
/// \todo Make this definition specific to windows.
#define NOMINMAX
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Core/Datatypes/Geometry.h>
#include <Interface/Modules/Render/share.h>



namespace SCIRun {

namespace Render {
  class QOSPRayWidget;
  class OSPRayRenderer;
  enum class MouseButton;
}

namespace Gui {

class ViewOspraySceneConfigDialog;

class SCISHARE OsprayViewerDialog : public ModuleDialogGeneric, public Ui::OsprayViewer
{
  Q_OBJECT;

public:
  OsprayViewerDialog(const std::string& name,
    Dataflow::Networks::ModuleStateHandle state,
    QWidget* parent = nullptr);
  ~OsprayViewerDialog();
  void adjustToolbar() override;

Q_SIGNALS:
  void newGeometryValueForwarder();

protected:
  virtual void pullSpecial() override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void contextMenuEvent(QContextMenuEvent* evt) override {}

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
  void addToolbarButton(QPushButton* button);
  void toggleLockColor(bool locked);
  void mousePositionToScreenSpace(int xIn, int yIn, float& xOut, float& yOut);
  Render::MouseButton getRenderButton(QMouseEvent* event);

  float getFloat(const Core::Algorithms::Name& name) const;

  Render::QOSPRayWidget* viewer_ {nullptr};

  Render::OSPRayRenderer* renderer_ {nullptr};

  QStatusBar* statusBar_ {nullptr};
  QToolBar* toolBar_ {nullptr};

  ViewOspraySceneConfigDialog* configDialog_;
  QAction* lockRotation_;
  QAction* lockPan_;
  QAction* lockZoom_;
  QPushButton* controlLock_;
  QPushButton* autoViewButton_;
  QPushButton* autoRotateButton_;
  QPushButton* playTimestepsButton_;
};
}}

#endif
