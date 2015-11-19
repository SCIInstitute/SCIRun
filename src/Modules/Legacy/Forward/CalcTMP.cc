/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2011 Scientific Computing and Imaging Institute,
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

//#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Module.h>

#include <Packages/BioPSE/Core/Algorithms/NumApproximation/CalcTMP.h>

//#include <algorithm>

namespace BioPSE {
  
using namespace SCIRun;

class CalcTMP : public Module, public CalcTMPAlgo {
  public:
    CalcTMP(GuiContext*);
    virtual ~CalcTMP() {}

    virtual void execute();
  
  private:
    //SCIRunAlgo::CalcTMPAlgo algo_;
    CalcTMPAlgo algo_;
};


DECLARE_MAKER(CalcTMP)

CalcTMP::CalcTMP(GuiContext* ctx)
  : Module("CalcTMP", ctx, Source, "Forward", "BioPSE")
{
  //algo_.set_progress_reporter(this);
}


void CalcTMP::execute()
{
  //FieldHandle Field;
  MatrixHandle amplitudes;
  MatrixHandle deps;
  MatrixHandle depslopes;
  MatrixHandle platslopes;
  MatrixHandle reps;
  MatrixHandle repslopes;
  MatrixHandle rests;
  MatrixHandle TMPs;
  
  //if (!(get_input_handle("Mesh",Field,true))) return;
  get_input_handle("Amplitude", amplitudes, false);
  get_input_handle("Depolarization Time", deps, false);
  get_input_handle("Depolarization Slope", depslopes, false);
  get_input_handle("Plateau Slope", platslopes, false);
  get_input_handle("Repolarization Time", reps, false);
  get_input_handle("Repolarization Slope", repslopes, false);
  get_input_handle("Rest Potential", rests, false);
  
  if (inputs_changed_ || !oport_cached("TMPs") )
  {
    //algo_.set_bool("generate_basis",gui_use_basis_.get());
    //algo_.set_bool("force_symmetry",gui_force_symmetry_.get());
    //if(!(algo_.run(Field,Conductivity,SysMatrix))) return;
    
    algo_.calc_TMPs(amplitudes,
                    deps,
                    depslopes,
                    platslopes,
                    reps,
                    repslopes,
                    rests,
                    500,
                    TMPs);
    
    send_output_handle("TMPs", TMPs);
  }
}

} // End namespace BioPSE



