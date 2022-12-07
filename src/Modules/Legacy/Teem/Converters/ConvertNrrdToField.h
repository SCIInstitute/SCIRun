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


#ifndef MODULES_TEEM_CONVERTERS_CONVERTNRRDTOFIELD_H
#define MODULES_TEEM_CONVERTERS_CONVERTNRRDTOFIELD_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Teem/Converters/share.h>

namespace SCIRun {

  namespace Core {
    namespace Algorithms {
      namespace Teem {
        ALGORITHM_PARAMETER_DECL(DataLocation);
        ALGORITHM_PARAMETER_DECL(FieldType);
        ALGORITHM_PARAMETER_DECL(ConvertParity);
      }}}

namespace Modules {
namespace Teem {

  class SCISHARE ConvertNrrdToField : public Dataflow::Networks::Module,
    public Has1InputPort<NrrdPortTag>,
    public Has1OutputPort<FieldPortTag>
  {
  public:
    ConvertNrrdToField();
    void execute() override;
    void setStateDefaults() override;

    INPUT_PORT(0, InputNrrd, NrrdDataType);
    OUTPUT_PORT(0, OutputField, Field);

    MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)
  };

}}}

#endif
