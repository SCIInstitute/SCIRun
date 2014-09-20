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
/// @todo Documentation Modules/Legacy/FiniteElements/BuildFEVolRHS.cc
/*
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED
 #include <Core/Datatypes/SparseRowMatrix.h>
 #include <Core/Datatypes/DenseMatrix.h>
 #include <Core/Datatypes/Matrix.h>
 #include <Core/Datatypes/Field.h>
 #include <Core/Datatypes/MatrixTypeConverter.h>

 #include <Dataflow/Network/Ports/MatrixPort.h>
 #include <Dataflow/Network/Ports/FieldPort.h>
 #include <Dataflow/GuiInterface/GuiVar.h>
 #include <Dataflow/Network/Module.h>
#endif
*/
#include <Modules/Legacy/FiniteElements/BuildFEVolRHS.h>
#include <Core/Algorithms/Legacy/FiniteElements/BuildRHS/BuildFEVolRHS.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Legacy/Field/Field.h>

using namespace SCIRun::Modules::FiniteElements;
using namespace SCIRun::Dataflow::Networks;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun;

BuildFEVolRHS::BuildFEVolRHS()
  : Module(ModuleLookupInfo("BuildFEVolRHS", "FiniteElements", "SCIRun"))
{
  INITIALIZE_PORT(Mesh);
  INITIALIZE_PORT(Vector_Table);
  INITIALIZE_PORT(RHS);
}

void BuildFEVolRHS::setStateDefaults()
{
// setStateBoolFromAlgo(SetFieldDataAlgo::keepTypeCheckBox);
}

void BuildFEVolRHS::execute()
{
  auto mesh = getRequiredInput(Mesh);
  auto vtable = getRequiredInput(Vector_Table);

  if (needToExecute())
  {
    auto output = algo().run_generic(make_input((Mesh, mesh)(Vector_Table, vtable)));
    sendOutputFromAlgorithm(RHS, output);
  }
 
 /* 
#ifdef SCIRUN4_ESSENTIAL_CODE_TO_BE_PORTED  
  FieldHandle Field;
  MatrixHandle VectorTable;
  MatrixHandle RHSMatrix;
  
  if (!(get_input_handle("Mesh",Field,true))) return;
  get_input_handle("Vector Table", VectorTable, false);
  
  if (inputs_changed_ || gui_use_basis_.changed() || !oport_cached("RHS") )
  {
    algo_.set_bool("generate_basis",gui_use_basis_.get());
    if(!(algo_.run(Field,VectorTable,RHSMatrix))) return;
    
    send_output_handle("RHS", RHSMatrix);
  }
#endif  
  
  auto mesh = getRequiredInput(Mesh);
  auto vectorTable = getRequiredInput(Vector_Table);
  
  if (needToExecute())
  {
    Core::Algorithms::AlgorithmOutput output; //= algo().run_generic(withInputData((Mesh, mesh)(Vector_Table, vectorTable)));
    sendOutputFromAlgorithm(RHS, output);
  }
 */ 
}



#ifdef SCIRUN4_CODE_TO_BE_ENABLED_LATER


#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/Field.h>
#include <Core/Datatypes/MatrixTypeConverter.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Module.h>

#include <Core/Algorithms/FiniteElements/BuildRHS/BuildFEVolRHS.h>


namespace SCIRun {

class BuildFEVolRHS : public Module {
  public:
    BuildFEVolRHS(GuiContext*);
    virtual ~BuildFEVolRHS() {}

    virtual void execute();

    GuiInt gui_use_basis_;
    //GuiInt gui_force_symmetry_;
  
  private:
    SCIRunAlgo::BuildFEVolRHSAlgo algo_;
};


DECLARE_MAKER(BuildFEVolRHS)

BuildFEVolRHS::BuildFEVolRHS(GuiContext* ctx)
  : Module("BuildFEVolRHS", ctx, Source, "FiniteElements", "SCIRun"),
    gui_use_basis_(get_ctx()->subVar("use-basis"),0)
{
  algo_.set_progress_reporter(this);
}


void BuildFEVolRHS::execute()
{
  FieldHandle Field;
  MatrixHandle VectorTable;
  MatrixHandle RHSMatrix;
  
  if (!(get_input_handle("Mesh",Field,true))) return;
  get_input_handle("Vector Table", VectorTable, false);
  
  if (inputs_changed_ || gui_use_basis_.changed() || !oport_cached("RHS") )
  {
    algo_.set_bool("generate_basis",gui_use_basis_.get());
    if(!(algo_.run(Field,VectorTable,RHSMatrix))) return;
    
    send_output_handle("RHS", RHSMatrix);
  }
}

} // End namespace SCIRun

#endif

