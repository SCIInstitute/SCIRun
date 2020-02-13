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

#ifndef MODULES_LEGACY_MATLAB_DATAIO_EXPORTMATRICESTOMATLAB_H
#define MODULES_LEGACY_MATLAB_DATAIO_EXPORTMATRICESTOMATLAB_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Matlab/DataIO/share.h>

namespace SCIRun {

  namespace Core
  {
    namespace Algorithms
    {
      namespace Matlab
      {
        ALGORITHM_PARAMETER_DECL(MatrixNames);
        ALGORITHM_PARAMETER_DECL(MatrixFormats);
      }
    }
  }

namespace Modules {
namespace Matlab {

  class SCISHARE ExportMatricesToMatlab : public Dataflow::Networks::Module,
    public Has2InputPorts<DynamicPortTag<MatrixPortTag>, StringPortTag>,
    public HasNoOutputPorts
  {
  public:
    ExportMatricesToMatlab();
    virtual void execute() override;
    virtual void setStateDefaults() override;
    INPUT_PORT_DYNAMIC(0, InputMatrix, Matrix);
    INPUT_PORT(1, Filename, String);
    HAS_DYNAMIC_PORTS

    MODULE_TRAITS_AND_INFO(ModuleHasUI)

    LEGACY_MATLAB_MODULE
  };
}}}

#endif
