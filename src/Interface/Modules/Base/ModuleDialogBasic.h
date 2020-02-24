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


#ifndef INTERFACE_APPLICATION_MODULE_DIALOG_BASIC_H
#define INTERFACE_APPLICATION_MODULE_DIALOG_BASIC_H

#include "Interface/Modules/Base/ui_ModuleDialogBasic.h"
#include "Interface/Modules/Base/ui_SubnetDialog.h"
#include <Interface/Modules/Base/ModuleDialogGeneric.h>
#include <Interface/Modules/Base/share.h>

namespace SCIRun {
namespace Gui {

  class SCISHARE ModuleDialogBasic : public ModuleDialogGeneric, public Ui::ModuleDialogBasic
  {
    Q_OBJECT

  public:
    explicit ModuleDialogBasic(const std::string& name, QWidget* parent = 0);
  };

  class SCISHARE SubnetDialog : public ModuleDialogGeneric, public Ui::Subnet
  {
    Q_OBJECT

  public:
    explicit SubnetDialog(const std::string& name, Dataflow::Networks::ModuleStateHandle state,
      QWidget* parent = nullptr);
    void pullSpecial() override;
  };
}
}

#endif
