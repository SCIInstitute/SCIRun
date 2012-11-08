/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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

#ifndef INTERFACE_APPLICATION_MODULE_DIALOG_GENERIC_H
#define INTERFACE_APPLICATION_MODULE_DIALOG_GENERIC_H

#include <QDialog>
#include <Dataflow/Network/NetworkFwd.h>
#include <Interface/Modules/Base/Share.h>

namespace SCIRun {
namespace Gui {
  
  class SCISHARE ModuleDialogGeneric : public QDialog
  {
    Q_OBJECT
  public:
    virtual ~ModuleDialogGeneric() {}
    //TODO: input state hookup?
    //yeah: eventually replace int with generic dialog state object, but needs to be two-way (set/get)
    //virtual int moduleExecutionTime() = 0;
    //TODO: how to genericize this?  do we need to?
  public Q_SLOTS:
    virtual void moduleExecuted() {}
    virtual void pull() = 0;
  Q_SIGNALS:
    void executionTimeChanged(int time);
    void executeButtonPressed();
  protected:
    explicit ModuleDialogGeneric(SCIRun::Dataflow::Networks::ModuleStateHandle state, QWidget* parent = 0);
    void fixSize();
    SCIRun::Dataflow::Networks::ModuleStateHandle state_;

    //TODO: need a better push/pull model
    bool pulling_;
    struct Pulling
    {
      explicit Pulling(ModuleDialogGeneric* m) : m_(m) { m->pulling_ = true; }
      ~Pulling() { m_->pulling_ = false; }
      ModuleDialogGeneric* m_;
    };
  };

}
}

#endif
