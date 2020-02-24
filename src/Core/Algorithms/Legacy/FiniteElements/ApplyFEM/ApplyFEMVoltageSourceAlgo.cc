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


/*
*  ApplyFEMVoltageSource.cc:  Builds the RHS of the FE matrix for voltage sources
*
*  Written by:
*   David Weinstein
*   University of Utah
*   May 1999
*  Modified by:
*   Alexei Samsonov, March 2001
*   Frank B. Sachse, February 2006
*
*/

#include <Core/Algorithms/Legacy/FiniteElements/ApplyFEM/ApplyFEMVoltageSourceAlgo.h>

#include <Core/Datatypes/Matrix.h>
#include <Core/Datatypes/DenseMatrix.h>
#include <Core/Datatypes/DenseColumnMatrix.h>
#include <Core/Datatypes/SparseRowMatrix.h>
#include <Core/Datatypes/MatrixTypeConversions.h>
#include <Core/Datatypes/Legacy/Field/Mesh.h>
#include <Core/Datatypes/Legacy/Field/VMesh.h>
#include <Core/Datatypes/Legacy/Field/VField.h>
#include <Core/Datatypes/Legacy/Field/Field.h>
#include <Core/Datatypes/Legacy/Field/FieldInformation.h>
#include <Core/Logging/LoggerInterface.h>

using namespace SCIRun;
using namespace SCIRun::Core::Algorithms;
using namespace SCIRun::Core::Algorithms::FiniteElements;
using namespace SCIRun::Core::Datatypes;
using namespace SCIRun::Core::Geometry;

ApplyFEMVoltageSourceAlgo::ApplyFEMVoltageSourceAlgo()
{

}


void ApplyFEMVoltageSourceAlgo::ExecuteAlgorithm(const DenseMatrixHandle& dirBC, DenseColumnMatrixHandle& rhs, SparseRowMatrixHandle& mat)
{
  //! adjusting matrix for Dirichlet BC
  index_type *idcNz;
  double *valNz;
  index_type idcNzsize;

  std::vector<double> dbc;
  index_type idx;
  size_type size = dirBC->nrows();
  for(idx = 0; idx<size; ++idx)
  {
    index_type ni = (*dirBC)(idx, 0);
    double val = (*dirBC)(idx, 1);

    // -- getting column indices of non-zero elements for the current row
    mat->getRowNonzerosNoCopy(ni, idcNzsize, idcNz, valNz);

    // -- updating rhs
    for (index_type i=0; i<idcNzsize; ++i)
    {
      index_type j = idcNz ? idcNz[i] : i;
      (*rhs)[j] += -val * valNz[i];
    }
  }

  //! zeroing matrix row and column corresponding to the dirichlet nodes
  size = dirBC->nrows();
  for(idx = 0; idx<size; ++idx)
  {
    index_type ni = (*dirBC)(idx, 0);
    double val = (*dirBC)(idx, 1);

    mat->getRowNonzerosNoCopy(ni, idcNzsize, idcNz, valNz);

    for (index_type i=0; i<idcNzsize; ++i)
    {
      index_type j = idcNz?idcNz[i]:i;
      mat->put(ni, j, 0.0);
      mat->put(j, ni, 0.0);
    }

    //! updating dirichlet node and corresponding entry in rhs
    mat->put(ni, ni, 1);
    (*rhs)[ni] = val;
  }
}
