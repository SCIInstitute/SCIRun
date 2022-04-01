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


#ifndef MODULES_TEEM_Tend_TendFiber_H
#define MODULES_TEEM_Tend_TendFiber_H

#include <Dataflow/Network/Module.h>
#include <Modules/Legacy/Teem/Tend/share.h>

namespace SCIRun::Core::Algorithms::Teem
{
  ALGORITHM_PARAMETER_DECL(FiberType);
  ALGORITHM_PARAMETER_DECL(Puncture);
  ALGORITHM_PARAMETER_DECL(Neighborhood);
  ALGORITHM_PARAMETER_DECL(StepSize);
  ALGORITHM_PARAMETER_DECL(Integration);
  ALGORITHM_PARAMETER_DECL(UseAniso);
  ALGORITHM_PARAMETER_DECL(AnisoMetric);
  ALGORITHM_PARAMETER_DECL(AnisoThreshold);
  ALGORITHM_PARAMETER_DECL(UseLength);
  ALGORITHM_PARAMETER_DECL(FiberLength);
  ALGORITHM_PARAMETER_DECL(UseSteps);
  ALGORITHM_PARAMETER_DECL(Steps);
  ALGORITHM_PARAMETER_DECL(UseConf);
  ALGORITHM_PARAMETER_DECL(ConfThreshold);
  ALGORITHM_PARAMETER_DECL(Kernel);
}

namespace SCIRun::Modules::Teem
{

  class SCISHARE TendFiber : public Dataflow::Networks::Module,
    public Has2InputPorts<NrrdPortTag, FieldPortTag>,
    public Has1OutputPort<FieldPortTag>
  {
  public:
    TendFiber();
    ~TendFiber();
    void execute() override;
    void setStateDefaults() override;

    INPUT_PORT(0, InputNrrd, NrrdDataType);
    INPUT_PORT(1, SeedPoints, Field);
    OUTPUT_PORT(0, Fibers, Field);

    MODULE_TRAITS_AND_INFO(ModuleFlags::ModuleHasUI)
  private:
    std::unique_ptr<class TendFiberImpl> impl_;
  };

}

#endif
