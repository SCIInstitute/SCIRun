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


/*
 *  ConvertMatrixType: Unary matrix operations -- transpose, negate
 *
 *  Written by:
 *   David Weinstein
 *   Department of Computer Science
 *   University of Utah
 *   June 1999
 *
 */

#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

#include <Dataflow/GuiInterface/GuiVar.h>
#include <Core/Util/StringUtil.h>
#include <iostream>
#include <sstream>

namespace SCIRun {

class ConvertMatrixType : public Module {
  GuiString oldtype_;
  GuiString newtype_;
  GuiString nrow_;
  GuiString ncol_;
public:
  ConvertMatrixType(GuiContext* ctx);
  virtual ~ConvertMatrixType() {}
  virtual void execute();
};

DECLARE_MAKER(ConvertMatrixType)
ConvertMatrixType::ConvertMatrixType(GuiContext* ctx)
: Module("ConvertMatrixType", ctx, Filter,"Math", "SCIRun"),
  oldtype_(get_ctx()->subVar("oldtype"), "Same"),
  newtype_(get_ctx()->subVar("newtype"), "Unknown"),
  nrow_(get_ctx()->subVar("nrow"), "??"),
  ncol_(get_ctx()->subVar("ncol"), "??")
{
}

void
ConvertMatrixType::execute()
{
  update_state(NeedData);

  MatrixHandle imH;
  get_input_handle("Input", imH, true);

  if (inputs_changed_ || !oport_cached("Output") || newtype_.changed())
  {
    nrow_.set(to_string(imH->nrows()));
    ncol_.set(to_string(imH->ncols()));

    oldtype_.set(imH->dynamic_type_name());

    std::string newtype = newtype_.get();
    MatrixHandle omH;

    if (newtype == "DenseMatrix") 
    {
      omH = imH->dense();
    } 
    else if (newtype == "DenseColMajMatrix") 
    {
      omH = imH->dense_col_maj();
    } 
    else if (newtype == "SparseRowMatrix") 
    {
      omH = imH->sparse();
    } 
    else if (newtype == "ColumnMatrix") 
    {
      omH = imH->column();
    } 
    else if (newtype == "Same") 
    {
      omH = imH;
    } 
    else 
    {
      error("ConvertMatrixType: unknown cast type "+newtype);
      return;
    }

    send_output_handle("Output", omH);
  }
}

} // End namespace SCIRun
