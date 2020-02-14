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


#ifndef MODULES_TEEM_MISC_BuildDerivedNrrdWithGage_H
#define MODULES_TEEM_MISC_BuildDerivedNrrdWithGage_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Teem/Misc/share.h>

namespace SCIRun {
  namespace Core {
    namespace Algorithms {
      namespace Teem {
        ALGORITHM_PARAMETER_DECL(FieldKind);
        ALGORITHM_PARAMETER_DECL(OType);
        ALGORITHM_PARAMETER_DECL(Quantity);
        ALGORITHM_PARAMETER_DECL(ValuesType);
        ALGORITHM_PARAMETER_DECL(ValuesNumParm1);
        ALGORITHM_PARAMETER_DECL(ValuesNumParm2);
        ALGORITHM_PARAMETER_DECL(ValuesNumParm3);
        ALGORITHM_PARAMETER_DECL(DType);
        ALGORITHM_PARAMETER_DECL(DNumParm1);
        ALGORITHM_PARAMETER_DECL(DNumParm2);
        ALGORITHM_PARAMETER_DECL(DNumParm3);
        ALGORITHM_PARAMETER_DECL(DDType);
        ALGORITHM_PARAMETER_DECL(DDNumParm1);
        ALGORITHM_PARAMETER_DECL(DDNumParm2);
        ALGORITHM_PARAMETER_DECL(DDNumParm3);
      }
    }
  }

namespace Modules {
namespace Teem {

  class SCISHARE BuildDerivedNrrdWithGage : public Dataflow::Networks::Module,
    public Has1InputPort<NrrdPortTag>,
    public Has1OutputPort<NrrdPortTag>
  {
  public:
    BuildDerivedNrrdWithGage();
    virtual void execute() override;
    virtual void setStateDefaults() override;

    INPUT_PORT(0, InputNrrd, NrrdDataType);
    OUTPUT_PORT(0, OutputNrrd, NrrdDataType);

    MODULE_TRAITS_AND_INFO(ModuleHasUI)
  };

}}}

#endif
