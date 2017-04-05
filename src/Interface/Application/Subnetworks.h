/*
 For more information, please see: http://software.sci.utah.edu

 The MIT License

 Copyright (c) 2015 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_APPLICATION_SUBNETWORKS_H
#define INTERFACE_APPLICATION_SUBNETWORKS_H

#include "ui_SubnetEditor.h"
#include <Dataflow/Network/ModuleDescription.h>
#include <QDockWidget>

namespace SCIRun
{
  namespace Gui
  {
    class NetworkEditor;

    class SubnetworkEditor : public QDockWidget, public Ui::SubnetEditor
    {
      Q_OBJECT

    public:
      SubnetworkEditor(NetworkEditor* editor, const SCIRun::Dataflow::Networks::ModuleId& subnetModuleId, const QString& name, QWidget* parent = nullptr);
      NetworkEditor* get() { return editor_; }
    private Q_SLOTS:
      void expand();
    private:
      NetworkEditor* editor_;
      SCIRun::Dataflow::Networks::ModuleId subnetModuleId_;
    };

  }
}

#endif
