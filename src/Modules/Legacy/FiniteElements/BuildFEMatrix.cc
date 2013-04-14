/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2009 Scientific Computing and Imaging Institute,
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

#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Util/StringUtil.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Module.h>

#include <Core/Algorithms/FiniteElements/BuildMatrix/BuildFEMatrix.h>


namespace SCIRun {

class BuildFEMatrix : public Module {
  public:
    BuildFEMatrix(GuiContext*);
    virtual ~BuildFEMatrix() {}

    virtual void execute();

    GuiInt gui_use_basis_;
    GuiInt gui_force_symmetry_;
    GuiString gui_num_processors_;
  
  private:
    SCIRunAlgo::BuildFEMatrixAlgo algo_;
};


DECLARE_MAKER(BuildFEMatrix)

BuildFEMatrix::BuildFEMatrix(GuiContext* ctx)
  : Module("BuildFEMatrix", ctx, Source, "FiniteElements", "SCIRun"),
    gui_use_basis_(get_ctx()->subVar("use-basis"), 0),
    gui_force_symmetry_(get_ctx()->subVar("force-symmetry"), 0),
    gui_num_processors_(get_ctx()->subVar("num-processors"), "auto")
{
  algo_.set_progress_reporter(this);
}


void BuildFEMatrix::execute()
{
  FieldHandle Field;
  MatrixHandle Conductivity;
  MatrixHandle SysMatrix;
  
  if (! get_input_handle("Mesh", Field, true) )
    return;

  get_input_handle("Conductivity Table", Conductivity, false);
  
  if (inputs_changed_ || gui_use_basis_.changed() || !oport_cached("Stiffness Matrix") )
  {
    algo_.set_bool("generate_basis", gui_use_basis_.get());
    algo_.set_bool("force_symmetry", gui_force_symmetry_.get());
    
    std::string num_proc_string = gui_num_processors_.get();
    int num_proc = SCIRunAlgo::BuildFEMatrixAlgo::AUTO;
    if ( (num_proc_string != "auto") && (! from_string(num_proc_string, num_proc) ) )
    {
      warning("'Number of Threads' GUI parameter could not be parsed. Using default number of threads.");
    }

    algo_.set_int("num_processors", num_proc);

    if (! algo_.run(Field,Conductivity,SysMatrix) )
      return;
    
    send_output_handle("Stiffness Matrix", SysMatrix);
  }
}

} // End namespace SCIRun



