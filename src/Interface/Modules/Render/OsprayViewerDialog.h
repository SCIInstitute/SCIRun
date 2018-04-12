/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2015 Scientific Computing and Imaging Institute,
University of Utah.

License for the specific language governing rights and limitations under
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

class OsprayObjectImpl;
class VolumeViewer;

namespace SCIRun {

  namespace Gui {

    class SCISHARE OsprayViewerDialog : public ModuleDialogGeneric, public Ui::OsprayViewer
    {
      Q_OBJECT

    public:
      OsprayViewerDialog(const std::string& name,
        Dataflow::Networks::ModuleStateHandle state,
        QWidget* parent = nullptr);
      ~OsprayViewerDialog();
      void adjustToolbar() override;
    Q_SIGNALS:
      void newGeometryValueForwarder();
    private Q_SLOTS:
      void newGeometryValue();
      void autoViewClicked();
      void autoRotateClicked();
      void screenshotClicked();
    private:
      void addToolBar();
      void addConfigurationButton();
      void addConfigurationDock();
      void addAutoViewButton();
      void addViewBarButton();
      void addAutoRotateButton();
      void addTimestepButtons();
      void addScreenshotButton();
      void addControlLockButton();
      void addToolbarButton(QPushButton* button);
      void toggleLockColor(bool locked);

      void createViewer(const Core::Datatypes::CompositeOsprayGeometryObject& obj);
      VolumeViewer* viewer_ {nullptr};
      boost::shared_ptr<OsprayObjectImpl> impl_;

      QToolBar* toolBar_{nullptr};
      //QToolBar*                             viewBar_;
      //QComboBox*                            mDownViewBox;
      //QComboBox*                            mUpVectorBox;
      //ViewSceneControlsDock*                mConfigurationDock;
      QAction* lockRotation_;
      QAction* lockPan_;
      QAction* lockZoom_;
      QPushButton* controlLock_;
      QPushButton* autoViewButton_;
      QPushButton* autoRotateButton_;
    };
  }
}

#endif
