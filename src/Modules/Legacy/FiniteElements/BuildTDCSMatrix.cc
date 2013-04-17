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

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Ports/FieldPort.h>
#include <Dataflow/GuiInterface/GuiVar.h>
#include <Dataflow/Network/Module.h>

#include <Core/Algorithms/FiniteElements/BuildMatrix/BuildTDCSMatrix.h>


namespace SCIRun {

class BuildTDCSMatrix : public Module {
  public:
    BuildTDCSMatrix(GuiContext*);
    virtual ~BuildTDCSMatrix() {}

    virtual void execute();
  
   private:
    SCIRunAlgo::BuildTDCSMatrix algo_;

};


DECLARE_MAKER(BuildTDCSMatrix)

BuildTDCSMatrix::BuildTDCSMatrix(GuiContext* ctx)
  : Module("BuildTDCSMatrix", ctx, Source, "FiniteElements", "SCIRun")
{
  algo_.set_progress_reporter(this);
}


void BuildTDCSMatrix::execute()
{
  FieldHandle Mesh;
  MatrixHandle Stiffness;
  MatrixHandle ElectrodeElements;
  MatrixHandle ElectrodeElementType;
  MatrixHandle ElectrodeElementDefinition;
  MatrixHandle ContactImpedance;
  MatrixHandle TDCSMatrix;  

  if (!(get_input_handle("FEM Stiffness",Stiffness,true))) return;
  if (!(get_input_handle("Mesh",Mesh,true))) return;
  if (!(get_input_handle("Electrode Element",ElectrodeElements,true))) return; 
  if (!(get_input_handle("Electrode Element Type",ElectrodeElementType,true))) return; 
  if (!(get_input_handle("Electrode Element Definition",ElectrodeElementDefinition,true))) return; 
  if (!(get_input_handle("Contact Impedance",ContactImpedance,true))) return;   
 
  algo_.run(Stiffness,Mesh,ElectrodeElements,ElectrodeElementType,ElectrodeElementDefinition,ContactImpedance,TDCSMatrix);  

  send_output_handle("TDCS Matrix", TDCSMatrix);
  
}

} // End namespace SCIRun



