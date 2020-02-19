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


#include <Core/Algorithms/Legacy/FiniteElements/Mapping/BuildNodeLink.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/SparseRowMatrixFromMap.h>
#include <Core/Datatypes/MatrixTypeConversions.h>

#include <Core/Math/MiscMath.h>
#include <boost/scoped_array.hpp>

using namespace SCIRun;
using namespace SCIRun::Core::Geometry;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Thread;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Logging;

bool
BuildNodeLinkAlgo::run(MatrixHandle nodeDomain, MatrixHandle& nodeLink) const
{
  ScopedAlgorithmStatusReporter asr(this, "BuildNodeLink");

  if (!nodeDomain)
  {
    error("No matrix on input");
    return (false);
  }

  if (nodeDomain->ncols() != 1)
  {
    error("NodeDomain needs to be a column vector");
    return (false);
  }

  DenseColumnMatrixHandle nodeDomainCol = convertMatrix::toColumn(nodeDomain);

  size_type num_values = nodeDomainCol->nrows();

  double min2 = -std::numeric_limits<double>::max();
  double min = std::numeric_limits<double>::max();

  bool found_domain = false;

  LegacySparseDataContainer<double> sparseData(num_values+1, num_values, 0);
  auto rr = sparseData.rows();
  auto cc = sparseData.columns();
  size_type nnz = 0;

  if (!rr || !cc)
  {
    error("Could not allocate memory for sparse matrix");
    return (false);
  }

  for (index_type j = 0; j<num_values; j++) cc[j] = -1;

  do
  {
    found_domain = false;
    min = std::numeric_limits<double>::max();

    for (index_type j = 0; j<num_values; j++)
    {
      double val = (*nodeDomainCol)(j);
      if (IsFinite(val))
      {
        if (val < min && val > min2 )
        {
          found_domain = true;
          min = val;
        }
      }
    }

    if (found_domain)
    {
      min2 = min;
      index_type idx = -1;
      for (index_type j = 0; j<num_values; j++)
      {
        double val = (*nodeDomainCol)(j);
        if (val == min2)
        {
          if (idx < 0) idx = j;
          else { cc[j] = idx; nnz++; }
        }
      }
    }
  }
  while (found_domain);

  sparseData.allocateData(nnz);
  auto aa = sparseData.data();

  if (!aa)
  {
    error("Could not allocate memory for sparse matrix");
    return (false);
  }

  index_type k = 0;
  rr[0] = 0;
  for (index_type j = 0; j<num_values; j++)
  {
    if (cc[j] > -1 )
    {
      cc[k] = cc[j];
      aa[k] = 1.0;
      k++;
    }
    rr[j+1] = k;
  }

  SparseRowMatrix nodeLinkHalf(num_values, num_values, rr.get(), cc.get(), aa.get(), nnz);
  SparseRowMatrix nodeLinkHalfT(nodeLinkHalf.transpose());
  nodeLink.reset(new SparseRowMatrix(nodeLinkHalf + nodeLinkHalfT));
  return (true);
}

AlgorithmOutput BuildNodeLinkAlgo::run(const AlgorithmInput &) const
{
  throw "not implemented";
}
