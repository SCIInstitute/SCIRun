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


#include <Core/Util/StringUtil.h>
#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/ColumnMatrix.h>
#include <Core/Datatypes/DenseColMajMatrix.h>
#include <Core/Datatypes/DenseMatrix.h>

#include <Dataflow/Network/Ports/MatrixPort.h>
#include <Dataflow/Network/Module.h>

namespace SCIRun {

using namespace SCIRun;

/// @class CreateMatrix
/// @brief This module lets the user create a small dense matrix manually.

class CreateMatrix : public Module
{
  public:
    CreateMatrix(GuiContext*);
    virtual ~CreateMatrix() {}

    virtual void execute();

  private:
    GuiInt    nrows_;
    GuiInt    ncols_;
    GuiString data_;
    GuiString clabel_;
    GuiString rlabel_;
};


DECLARE_MAKER(CreateMatrix)

CreateMatrix::CreateMatrix(GuiContext* ctx) :
  Module("CreateMatrix", ctx, Source, "Math", "SCIRun"),
  nrows_(get_ctx()->subVar("rows"), 1),
  ncols_(get_ctx()->subVar("cols"), 1),
  data_(get_ctx()->subVar("data"), "{0.0}"),
  clabel_(get_ctx()->subVar("clabel"), "{0}"),
  rlabel_(get_ctx()->subVar("rlabel"), "{0}")
{
}

void
CreateMatrix::execute()
{
  MatrixHandle handle;
  TCLInterface::execute(get_id() + " update_matrixdata");

  size_type nrows = static_cast<size_type>(nrows_.get());
  size_type ncols = static_cast<size_type>(ncols_.get());
  std::string data = data_.get();

  MatrixHandle mat = new DenseColMajMatrix(nrows,ncols);

  if (mat.get_rep() == 0)
  {
    error("Could allocate output matrix");
    return;
  }

  for (size_t p=0;p<data.size();p++)
  {
    if ((data[p] == '}')||(data[p] == '{')) data[p] = ' ';
  }

  std::vector<double> nums;
  multiple_from_string(data,nums);

  double *ptr = mat->get_data_pointer();

  /// @todo: quick bug fix, this module is due for rewrite.
  for (index_type p = 0; p < nums.size(); p++)
  {
    ptr[p] = nums[p];
  }

  handle = mat->dense();
  send_output_handle("matrix", handle);
}

} // End namespace SCIRun
