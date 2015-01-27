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

#ifndef INTERFACE_MODULES_VIEW_SCENE_CONTROLS_H
#define INTERFACE_MODULES_VIEW_SCENE_CONTROLS_H

#include "Interface/Modules/Render/ui_ViewSceneControls.h"

#include <boost/shared_ptr.hpp>

#include <Interface/Modules/Render/ViewScene.h>

//#include <Modules/Basic/SendScalarModuleState.h>
//#include <Interface/Modules/Base/ModuleDialogGeneric.h>

#include <Interface/Modules/Render/namespaces.h>
#include <Interface/Modules/Render/share.h>

namespace SCIRun {
  namespace Gui {
    class ViewSceneDialog;

    class SCISHARE ViewSceneControlsDock : public QDockWidget, public Ui::ViewSceneControls
    {
      Q_OBJECT

    public:
      ViewSceneControlsDock(const QString& name, ViewSceneDialog* parent);

      void setSampleColor(const QColor& color);

    private:
      //ViewSceneDialog* scene_;

    };

  }
}

#endif //INTERFACE_MODULES_VIEW_SCENE_CONTROLS_H
