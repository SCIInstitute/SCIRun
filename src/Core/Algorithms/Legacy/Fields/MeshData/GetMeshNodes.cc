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


#include <Core/Algorithms/Legacy/Fields/MeshData/GetMeshNodes.h>
#include <Core/Algorithms/Base/AlgorithmPreconditions.h>
#include <Core/Algorithms/Base/AlgorithmVariableNames.h>

#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/DenseMatrix.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms::Fields;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Utility;
using namespace SCIRun::Core::Algorithms;

bool GetMeshNodesAlgo::run(FieldHandle& input, DenseMatrixHandle& output) const
{
  ScopedAlgorithmStatusReporter asr(this, "GetMeshNodes");

  if (!input)
  {
    error("No input source field");
    return (false);
  }

  VMesh* vmesh = input->vmesh();
  VMesh::size_type size = vmesh->num_nodes();

  output.reset(new DenseMatrix(size,3));

  if (!output)
  {
    error("Could not allocate output matrix");
    return (false);
  }

  FieldInformation fi(input);

  /// @todo: refactor duplication
  if (fi.is_regularmesh())
  {
    Point p;
    int cnt = 0;
    for (VMesh::Node::index_type i=0; i<size; ++i)
    {
      vmesh->get_center(p,i);
      (*output)(i, 0) = p.x();
      (*output)(i, 1) = p.y();
      (*output)(i, 2) = p.z();
      cnt++;
      if (cnt == 400)
      {
        cnt = 0;
        update_progress_max(i,size);
      }
    }

  }
  else
  {
    Point*  points  = vmesh->get_points_pointer();

    Point p;
    int cnt = 0;
    for (VMesh::Node::index_type i=0; i<size; ++i)
    {
      p = points[i];
      (*output)(i, 0) = p.x();
      (*output)(i, 1) = p.y();
      (*output)(i, 2) = p.z();
      cnt++;
      if (cnt == 400)
      {
        cnt = 0;
        update_progress_max(i,size);
      }
    }
  }

  return (true);
}

AlgorithmOutputName GetMeshNodesAlgo::MatrixNodes("MatrixNodes");

AlgorithmOutput GetMeshNodesAlgo::run(const AlgorithmInput& input) const
{
  auto inputField = input.get<Field>(Variables::InputField);
  DenseMatrixHandle nodes;
  if (!run(inputField, nodes))
    THROW_ALGORITHM_PROCESSING_ERROR("False returned on legacy run call.");

  AlgorithmOutput output;
  output[MatrixNodes] = nodes;
  return output;
}
