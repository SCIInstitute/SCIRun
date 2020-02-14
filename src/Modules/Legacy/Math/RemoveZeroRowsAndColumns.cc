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


/// @todo Documentation Modules/Legacy/Math/RemoveZeroRowsAndColumns.cc

#include <Core/Algorithms/Math/SelectMatrix/SelectSubMatrix.h>
#include <Core/Algorithms/Math/FindMatrix/FindMatrix.h>
#include <Core/Algorithms/Math/MappingMatrix/ConvertMappingOrderIntoMappingMatrix.h>

#include <Dataflow/Network/Module.h>
#include <Dataflow/Network/Ports/MatrixPort.h>



namespace SCIRun {

using namespace SCIRun;

class RemoveZeroRowsAndColumns : public Module {
  public:
    RemoveZeroRowsAndColumns(GuiContext*);

    virtual void execute();

  private:
    SCIRunAlgo::SelectSubMatrixAlgo select_submatrix_;
    SCIRunAlgo::FindMatrixAlgo find_algo_;
    SCIRunAlgo::ConvertMappingOrderIntoMappingMatrixAlgo convert_algo_;
};


DECLARE_MAKER(RemoveZeroRowsAndColumns)

RemoveZeroRowsAndColumns::RemoveZeroRowsAndColumns(GuiContext* ctx) :
  Module("RemoveZeroRowsAndColumns", ctx, Source, "Math", "SCIRun")
{
  select_submatrix_.set_progress_reporter(this);
  find_algo_.set_progress_reporter(this);
  convert_algo_.set_progress_reporter(this);
}

void
RemoveZeroRowsAndColumns::execute()
{
  MatrixHandle input, output, leftmapping, rightmapping;

  get_input_handle("Matrix",input,true);

  if (inputs_changed_ || !oport_cached("ReducedMatrix") ||
    !oport_cached("LeftMapping")||!oport_cached("RightMapping"))
  {
    std::vector<index_type> rows;
    std::vector<index_type> columns;

    find_algo_.set_option("method","nonzero_rows");
    find_algo_.run(input,rows);
    find_algo_.set_option("method","nonzero_columns");
    find_algo_.run(input,columns);
    select_submatrix_.run(input,output,rows,columns);
    convert_algo_.set_bool("transpose",true);
    convert_algo_.run(rows,leftmapping,input->nrows());
    convert_algo_.set_bool("transpose",false);
    convert_algo_.run(columns,rightmapping,input->nrows());

    send_output_handle("ReducedMatrix",output,true);
    send_output_handle("LeftMapping",leftmapping,true);
    send_output_handle("RightMapping",rightmapping,true);
  }
}

} // End namespace SCIRun
