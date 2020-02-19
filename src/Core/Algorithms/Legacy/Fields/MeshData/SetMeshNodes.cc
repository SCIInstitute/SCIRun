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


#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Algorithms/Legacy/Fields/MeshData/SetMeshNodes.h>
#include <Core/Algorithms/Legacy/Fields/ConvertMeshType/ConvertMeshToIrregularMesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/PropertyManagerExtensions.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Datatypes;

bool
SetMeshNodesAlgo::run(FieldHandle input, DenseMatrixHandle matrix, FieldHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "SetMeshNodes");

  if (!input)
  {
    error("No input source field");
    return (false);
  }

  if (!matrix)
  {
    error("No input source matrix");
    return (false);
  }

  VMesh::size_type numnodes = input->vmesh()->num_nodes();

  // try to see whether the matrix dimensions fit the mesh size
  if (!(matrix->nrows() == numnodes) ||
      !(matrix->ncols() == 3))
  {
    error("Matrix dimensions do not match any of the fields dimensions");
    return (false);
  }

  FieldInformation fi(input);
  if (fi.is_regularmesh())
  {
    /// @todo: worth separating out into factory call for mocking purposes? probably not, just keep the concrete dependence
    ConvertMeshToIrregularMeshAlgo algo;

    if (!algo.runImpl(input,output))
      return (false);
  }
  else
  {
    output.reset(input->deep_clone());
  }

  CopyProperties(*input, *output);

  VMesh* mesh = output->vmesh();
  VMesh::size_type size = mesh->num_nodes();

  Point p;
  int cnt =0;
  for (VMesh::Node::index_type i=0; i<size; ++i)
  {
    p.x( (*matrix)(i, 0) );
    p.y( (*matrix)(i, 1) );
    p.z( (*matrix)(i, 2) );

    mesh->set_point(p,i);
    cnt++; if (cnt == 400) {cnt=0; update_progress_max(i,size); }
  }

  return (true);
}

AlgorithmInputName SetMeshNodesAlgo::MatrixNodes("MatrixNodes");
AlgorithmOutputName SetMeshNodesAlgo::OutputField("OutputField");

AlgorithmOutput SetMeshNodesAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  auto nodes = input.get<DenseMatrix>(MatrixNodes);

  FieldHandle outputField;
  if (!run(inputField, nodes, outputField))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[OutputField] = outputField;
  return output;
}
