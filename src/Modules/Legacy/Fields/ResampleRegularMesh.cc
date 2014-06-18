/*
 * The MIT License
 *
 * Copyright (c) 2009 Scientific Computing and Imaging Institute,
 * University of Utah.
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <Modules/Legacy/Fields/ResampleRegularMesh.h>

//#include <Core/Util/StringUtil.h>

#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>

//#include <Core/Algorithms/Fields/ResampleMesh/ResampleRegularMesh.h>

using namespace SCIRun::Modules::Fields;
//using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
namespace SCIRun {

using namespace SCIRun;

/// @class ResampleRegularMesh
/// @brief Resample a regular mesh, such as a LatVol.

class ResampleRegularMesh : public Module {
  public:
    ResampleRegularMesh(GuiContext*);
    virtual ~ResampleRegularMesh() {}

    virtual void execute();

  private:
    GuiString method_;
    GuiDouble sigma_;
    GuiDouble extend_;
    GuiString xaxis_;
    GuiString yaxis_;
    GuiString zaxis_;

    SCIRunAlgo::ResampleRegularMeshAlgo algo_;
};


DECLARE_MAKER(ResampleRegularMesh)
#endif

ResampleRegularMesh::ResampleRegularMesh() :
  Module(ModuleLookupInfo("ResampleRegularMesh", "ChangeMesh", "SCIRun"))
{
  INITIALIZE_PORT(InputField);
  INITIALIZE_PORT(OutputField);
}

void ResampleRegularMesh::setStateDefaults()
{
  auto state = get_state();
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
    method_(get_ctx()->subVar("method"),"box"),
    sigma_(get_ctx()->subVar("sigma"),1.0),
    extend_(get_ctx()->subVar("extend"),1.0),
    xaxis_(get_ctx()->subVar("xdim"),"x0.5"),
    yaxis_(get_ctx()->subVar("ydim"),"x0.5"),
    zaxis_(get_ctx()->subVar("zdim"),"x0.5")
  #endif
}

void
ResampleRegularMesh::execute()
{
  #ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER
  FieldHandle input, output;

  get_input_handle("Field",input,true);

  if (inputs_changed_ || method_.changed() || sigma_.changed() ||
      extend_.changed() || xaxis_.changed() || yaxis_.changed() ||
      zaxis_.changed() || !oport_cached("Field"))
  {
    algo_.set_option("method",method_.get());
    algo_.set_scalar("sigma",sigma_.get());
    algo_.set_scalar("extend",extend_.get());

    std::string xaxis = xaxis_.get();
    std::string::size_type xloc = xaxis.find("x");

    algo_.set_option("resamplex","none");
    if (xloc != std::string::npos)
    {
      double factor = 0.0;
      from_string(xaxis.substr(xloc+1),factor);
      if (factor <= 0.0)
      {
        error("X axis has an incorrect factor");
        return;
      }
      algo_.set_option("resamplex","factor");
      algo_.set_scalar("xfactor",factor);
    }
    else
    {
      int number = 0;
      from_string(xaxis,number);
      if (number < 2)
      {
        error("X axis has an incorrect dimension");
        return;
      }
      algo_.set_option("resamplex","number");
      algo_.set_int("xnumber",number);
    }

    std::string yaxis = yaxis_.get();
    std::string::size_type yloc = yaxis.find("x");

    algo_.set_option("resampley","none");
    if (yloc != std::string::npos)
    {
      double factor = 0.0;
      from_string(yaxis.substr(yloc+1),factor);
      if (factor <= 0.0)
      {
        error("Y axis has an incorrect factor");
        return;
      }
      algo_.set_option("resampley","factor");
      algo_.set_scalar("yfactor",factor);
    }
    else
    {
      int number = 0;
      from_string(yaxis,number);
      if (number < 2)
      {
        error("Y axis has an incorrect dimension");
        return;
      }
      algo_.set_option("resampley","number");
      algo_.set_int("ynumber",number);
    }


    std::string zaxis = zaxis_.get();
    std::string::size_type zloc = zaxis.find("x");

    algo_.set_option("resamplez","none");
    if (zloc != std::string::npos)
    {
      double factor = 0.0;
      from_string(zaxis.substr(zloc+1),factor);
      if (factor <= 0.0)
      {
        error("Z axis has an incorrect factor");
        return;
      }
      algo_.set_option("resamplez","factor");
      algo_.set_scalar("zfactor",factor);
    }
    else
    {
      int number = 0;
      from_string(zaxis,number);
      if (number < 2)
      {
        error("Z axis has an incorrect dimension");
        return;
      }
      algo_.set_option("resamplez","number");
      algo_.set_int("znumber",number);
    }

    if(!(algo_.run(input,output))) return;

    send_output_handle("Field",output,true);
  }
  #endif
}
