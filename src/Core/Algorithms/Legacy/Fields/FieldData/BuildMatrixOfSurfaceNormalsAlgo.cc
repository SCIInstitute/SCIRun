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


#include <Core/Algorithms/Legacy/Fields/FieldData/BuildMatrixOfSurfaceNormalsAlgo.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/GeometryPrimitives/Vector.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>

using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Logging;
using namespace SCIRun::Core::Utility;
using namespace SCIRun;

BuildMatrixOfSurfaceNormalsAlgo::BuildMatrixOfSurfaceNormalsAlgo() {}

bool BuildMatrixOfSurfaceNormalsAlgo::runImpl(FieldHandle input, DenseMatrixHandle& output) const
{
	ScopedAlgorithmStatusReporter asr(this, "SetFieldDataToConstantValue");

  if (!input)
  {
    error("No input source field");
    return (false);
  }

  FieldInformation fi(input);
  if (!(fi.is_surface()))
  {
    error("This algorithm only works on a surface mesh");
    return (false);
  }

  VMesh* vmesh = input->vmesh();
  vmesh->synchronize(Mesh::NORMALS_E);
  VMesh::size_type num_nodes = vmesh->num_nodes();

  output.reset(new DenseMatrix(num_nodes, 3));

  if (!output)
  {
    error("Could not allocate output matrix");
    return (false);
  }

	VMesh::Node::index_type k = 0;
  int cnt = 0;
  Vector norm;
  for (VMesh::Node::index_type i=0; i<num_nodes; ++i)
  {
    vmesh->get_normal(norm,i);
    (*output)(k) = norm.x();
		(*output)(k+1) = norm.y();
		(*output)(k+2) = norm.z();
    k += 3;
    cnt++; if (cnt == 400) {cnt=0; update_progress_max(i,num_nodes); }
  }
  return (true);
}

AlgorithmOutput BuildMatrixOfSurfaceNormalsAlgo::run(const AlgorithmInput& input) const
{
  auto field = input.get<Field>(Variables::InputField);
	DenseMatrixHandle outputMatrix;

	if(!runImpl(field, outputMatrix))
			THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy call.");

  AlgorithmOutput output;
  output[Variables::OutputMatrix] = outputMatrix;
  return output;
}
