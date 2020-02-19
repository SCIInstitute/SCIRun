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


/// @author
///    McKay Davis
///    School of Computing
///    University of Utah
/// @date  August, 2005

#include <Dataflow/Network/Module.h>

#include <Core/Datatypes/NrrdData.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConverter.h>
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

namespace SCIRun {

using namespace SCIRun;

/// @class ConvertMappingMatrixToMaskVector
/// @brief This module converts a Mapping Matrix to a Mask Vector.

class ConvertMappingMatrixToMaskVector : public Module {
public:
  ConvertMappingMatrixToMaskVector(GuiContext*);
  virtual ~ConvertMappingMatrixToMaskVector();
  virtual void	execute();
};


DECLARE_MAKER(ConvertMappingMatrixToMaskVector)
ConvertMappingMatrixToMaskVector::ConvertMappingMatrixToMaskVector(GuiContext* ctx)
  : Module("ConvertMappingMatrixToMaskVector", ctx, Source, "Math", "SCIRun")
{
}

ConvertMappingMatrixToMaskVector::~ConvertMappingMatrixToMaskVector()
{
}

void
ConvertMappingMatrixToMaskVector::execute()
{
  update_state(Module::NeedData);

  MatrixHandle matrixH;
  if (!get_input_handle("MappingMatrix", matrixH)) return;

  SparseRowMatrix *matrix = matrix_cast::as_sparse(matrixH);
  if (!matrix)
    throw "Input matrix not sparse";

  size_t dim[NRRD_DIM_MAX];
  dim[0] = matrix->ncols();

  NrrdDataHandle nrrdH = new NrrdData;
  Nrrd *nrrd = nrrdH->nrrd_;
  nrrdAlloc_nva(nrrd, nrrdTypeUChar, 1, dim);
  unsigned char *mask = (unsigned char *)nrrd->data;
  memset(mask, 0, dim[0]*sizeof(unsigned char));
  index_type *rr = matrix->get_rows();
  index_type *cc = matrix->get_cols();
  double *data = matrix->get_vals();
  for (index_type i = 0; i < matrix->nrows(); ++i)
  {
    if (rr[i+1] == rr[i]) continue; // No entires on this row
    index_type col = cc[rr[i]];
    if (data[rr[i]] > 0.0)
    {
      mask[col] = 1;
    }
  }

  send_output_handle("MaskVector", nrrdH);
}

} // End namespace SCIRun
