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
#include <Dataflow/Network/Ports/NrrdPort.h>
#include <Dataflow/Network/Ports/MatrixPort.h>

namespace SCIRun {

using namespace SCIRun;

/// @class ConvertMaskVectorToMappingMatrix
/// @brief This module converts a Mask Vector to a Mapping Matrix.

class ConvertMaskVectorToMappingMatrix : public Module {
public:
  ConvertMaskVectorToMappingMatrix(GuiContext*);
  virtual ~ConvertMaskVectorToMappingMatrix();
  virtual void	execute();
};


DECLARE_MAKER(ConvertMaskVectorToMappingMatrix)
ConvertMaskVectorToMappingMatrix::ConvertMaskVectorToMappingMatrix(GuiContext* ctx)
  : Module("ConvertMaskVectorToMappingMatrix", ctx, Source, "Math", "SCIRun")
{
}

ConvertMaskVectorToMappingMatrix::~ConvertMaskVectorToMappingMatrix()
{
}

void
ConvertMaskVectorToMappingMatrix::execute()
{
  update_state(Module::JustStarted);
  update_state(Module::NeedData);

  NrrdDataHandle nrrdH;
  if (!get_input_handle("MaskVector", nrrdH)) return;

  Nrrd *nrrd = nrrdH->nrrd_;
  if (!nrrd)
    throw "Input MaskVector Nrrd empty";

  if (nrrd->type != nrrdTypeUChar)
    throw "Input MaskVector not Unsigned Char";


  const size_type dim = static_cast<size_type>(nrrd->axis[0].size);
  SparseRowMatrix::Data sparseData(dim+1, dim);
  const SparseRowMatrix::Rows& rr = sparseData.rows();
  const SparseRowMatrix::Columns& cc = sparseData.columns();
  const SparseRowMatrix::Storage& data = sparseData.data();
  const unsigned char *mask = (const unsigned char *)nrrd->data;
  for (index_type i = 0; i < dim; ++i) {
    rr[i] = i;
    cc[i] = i;
    data[i] = mask[i]?1.0:0.0;
  }

  rr[dim] = dim;

  MatrixHandle mtmp(new SparseRowMatrix(dim, dim, sparseData, dim));
  send_output_handle("MappingMatrix", mtmp);
}

} // End namespace SCIRun
