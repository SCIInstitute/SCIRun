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

#include "Interface/Modules/Render/ui_ClippingPlanes.h"

#include <Interface/Modules/Render/share.h>

class QwtKnob;
class ctkColorPickerButton;
class QToolBar;
class ctkPopupWidget;

namespace SCIRun {
namespace Gui {
    class ViewSceneVtkDialog;

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

    class SCISHARE ClippingPlaneControlsVtk : public ViewSceneVtkControlPopupWidget, public Ui::ClippingPlanes, public ButtonStylesheetToggler
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
}
}