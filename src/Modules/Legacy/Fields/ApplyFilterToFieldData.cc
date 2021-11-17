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

#include <Modules/Legacy/Fields/ApplyFilterToFieldData.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Algorithms/Legacy/Fields/FilterFieldData/DilateFieldData.h>
#include <Core/Algorithms/Legacy/Fields/FilterFieldData/ErodeFieldData.h>

using namespace SCIRun;
using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Dataflow::Networks;

MODULE_INFO_DEF(ApplyFilterToFieldData, ChangeFieldData, SCIRun)

ApplyFilterToFieldData::ApplyFilterToFieldData() :
  Module(staticInfo_)
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

/// @class ApplyFilterToFieldData
/// @brief Applies a dilate or erode filter to a regular mesh.

// class  : public Module {
//   public:
//     ApplyFilterToFieldData(GuiContext*);
//     virtual ~ApplyFilterToFieldData() {}
//     virtual void execute();
//
//   private:
//     GuiString method_;
//     GuiString edmethod_;
//     GuiInt edniter_;
//
//     SCIRunAlgo::ErodeFieldDataAlgo  erode_algo_;
//     SCIRunAlgo::DilateFieldDataAlgo dilate_algo_;
// };
//

// DECLARE_MAKER(ApplyFilterToFieldData)
//
// ApplyFilterToFieldData::ApplyFilterToFieldData(GuiContext* ctx) :
//   Module("ApplyFilterToFieldData", ctx, Source, "ChangeFieldData", "SCIRun"),
//     method_(ctx->subVar("method")),
//     edmethod_(ctx->subVar("ed-method")),
//     edniter_(ctx->subVar("ed-iterations"))
// {
//   erode_algo_.set_progress_reporter(this);
//   dilate_algo_.set_progress_reporter(this);
// }

void ApplyFilterToFieldData::setStateDefaults()
{
  setStateIntFromAlgo(Core::Algorithms::Variables::MaxIterations);
}

void
ApplyFilterToFieldData::execute()
{
  auto input = getRequiredInput(InputField);

  if (needToExecute())
  {
    #if 0
    if (method_.get() == "erodedilate")
    {
      if (edmethod_.get() == "erode")
      {
        erode_algo_.set_int("num_iterations",edniter_.get());
        if(!(erode_algo_.run(input,output))) return;
      }
      else  if (edmethod_.get() == "dilate")
      {
        dilate_algo_.set_int("num_iterations",edniter_.get());
        if(!(dilate_algo_.run(input,output))) return;
      }
      else
      {
        erode_algo_.set_int("num_iterations",edniter_.get());
        dilate_algo_.set_int("num_iterations",edniter_.get());
        if(!(erode_algo_.run(input,output))) return;
        if(!(dilate_algo_.run(output,output))) return;
      }
    }
    #endif
    //sendOutputFromAlgorithm(OutputField);
  }
}
