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
/// @todo Documentation Modules/Legacy/Fields/ScaleFieldMeshAndData.cc

#include <Modules/Legacy/Fields/ScaleFieldMeshAndData.h>

#include <Core/Algorithms/Legacy/Fields/TransformMesh/ScaleFieldMeshAndData.h>

using namespace SCIRun::Modules::Fields;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
namespace SCIRun {

class ScaleFieldMeshAndData : public Module {
  public:
    ScaleFieldMeshAndData(GuiContext*);
    virtual ~ScaleFieldMeshAndData() {}
    virtual void execute();

  private:
    GuiDouble guidatascale_;
    GuiDouble guigeomscale_;   
    GuiInt    guiusegeomcenter_;  
    
    SCIRunAlgo::ScaleFieldMeshAndDataAlgo algo_;
};
#endif

ModuleLookupInfo ScaleFieldMeshAndData::staticInfo_("ScaleFieldMeshAndData", "ChangeMesh", "SCIRun");

ScaleFieldMeshAndData::ScaleFieldMeshAndData()
  : Module(staticInfo_)
    //guidatascale_(ctx->subVar("datascale")),
    //guigeomscale_(ctx->subVar("geomscale")),
    //guiusegeomcenter_(ctx->subVar("usegeomcenter"))
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(GeomScaleFactor);
  INITIALIZE_PORT(DataScaleFactor);
  INITIALIZE_PORT(OutputField);
}

void ScaleFieldMeshAndData::setStateDefaults()
{
  auto state = get_state();

  //state->setValue(Parameters::ResampleMethod, std::string("box"));
  //setStateDoubleFromAlgo(Parameters::ResampleGaussianSigma);
  //setStateDoubleFromAlgo(Parameters::ResampleGaussianExtend);
  //setStateDoubleFromAlgo(Parameters::ResampleXDim);
  //setStateDoubleFromAlgo(Parameters::ResampleYDim);
  //setStateDoubleFromAlgo(Parameters::ResampleZDim);
  //setStateBoolFromAlgo(Parameters::ResampleXDimUseScalingFactor);
  //setStateBoolFromAlgo(Parameters::ResampleYDimUseScalingFactor);
  //setStateBoolFromAlgo(Parameters::ResampleZDimUseScalingFactor);
}

void ScaleFieldMeshAndData::execute()
{
#if 0
  FieldHandle input, output;
  MatrixHandle DataScale,GeomScale;
  
  get_input_handle("Field",input,true);
  get_input_handle("DataScaleFactor",DataScale,false);
  get_input_handle("GeomScaleFactor",GeomScale,false);
  
  if (inputs_changed_ || guidatascale_.changed() || guigeomscale_.changed() || 
      guiusegeomcenter_.changed() || !oport_cached("Field"))
  {
    update_state(Executing);

    if (DataScale.get_rep() && DataScale->ncols() > 0 && DataScale->nrows() > 0) 
    {
      double datascale = DataScale->get(0,0);
      guidatascale_.set(datascale);
      get_ctx()->reset();
    }

    if (GeomScale.get_rep() && GeomScale->ncols() > 0 && GeomScale->nrows() > 0) 
    {
      double geomscale = GeomScale->get(0,0);
      guigeomscale_.set(geomscale);
      get_ctx()->reset();
    }

    algo_.set_scalar("data_scale",guidatascale_.get());
    algo_.set_scalar("mesh_scale",guigeomscale_.get());
    algo_.set_bool("scale_from_center",guiusegeomcenter_.get());
    if(!(algo_.run(input, output))) return;
    
    send_output_handle("Field", output);
  }
#endif
}
